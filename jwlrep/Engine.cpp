// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Engine.h>

#include <jwlrep/AppConfig.h>
#include <jwlrep/Base64.h>
#include <jwlrep/EnumUtil.h>
#include <jwlrep/IEngineEventHandler.h>
#include <jwlrep/Logger.h>
#include <jwlrep/NetUtil.h>
#include <jwlrep/Report.h>
#include <jwlrep/RootCertificates.h>
#include <jwlrep/Worklog.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

#include <boost/fiber/asio/round_robin.hpp>
#include <boost/fiber/asio/yield.hpp>

#include <cassert>
#include <utility>

namespace {

constexpr std::size_t kBufferCapacity = 1024;

} // namespace
namespace jwlrep {

Engine::Engine(std::shared_ptr<boost::asio::io_context> ioContext,
               IEngineEventHandler& engineEventHandler,
               AppConfig const& appConfig)
    : ioContext_(ioContext),
      engineEventHandler_(engineEventHandler),
      appConfig_(appConfig),
      channel_(std::make_unique<boost::fibers::buffered_channel<int>>(
          kBufferCapacity)) {
  SPDLOG_DEBUG("Engine has been created.");
  assert(ioContext_);
  boost::fibers::use_scheduling_algorithm<boost::fibers::asio::round_robin>(
      ioContext_);
}

Engine::~Engine() {
  ioContext_->stop();
  SPDLOG_DEBUG("Engine has been destroyed.");
}

void Engine::start() {
  SPDLOG_DEBUG("Starting engine");
  assert(initiated_);

  sslContext_ = std::make_unique<boost::asio::ssl::context>(
      boost::asio::ssl::context::tlsv12_client);

  loadRootCertificates(*sslContext_);

  sslContext_->set_verify_mode(boost::asio::ssl::verify_peer);

  boost::fibers::fiber([this]() {
    SPDLOG_DEBUG("Launched main fiber");
    try {
      engineEventHandler_.onEngineStarted();

      // ---------------
      queryTimesheets();
      // ---------------

      // decltype(channel_)::element_type::value_type value;
      // while (boost::fibers::channel_op_status::success ==
      //       channel_->pop(value)) {

      // SPDLOG_DEBUG("Popped: {}", value);
      //}

    } catch (std::exception const& e) {
      SPDLOG_ERROR("Got exception in main fiber: {}", e.what());
    }

    engineEventHandler_.onEngineStopped();

    ioContext_->stop();

    SPDLOG_DEBUG("Finished main fiber");
  }).detach();

  SPDLOG_DEBUG("Engine has been launched.");

  return;
}

void Engine::stop() {
  SPDLOG_DEBUG("Stopping engine");
  assert(initiated_);

  if (channel_->is_closed()) {
    SPDLOG_DEBUG("Already stopped. Skip.");
    return;
  }
  channel_->push(100);
  channel_->close();

  return;
}

bool Engine::init() {
  assert(!initiated_);

  initiated_ = true;
  return initiated_;
}

void Engine::queryTimesheets() {
  namespace http = boost::beast::http;
  using tcp = boost::asio::ip::tcp;
  auto& yield = boost::fibers::asio::this_yield();
  namespace beast = boost::beast;
  namespace net = boost::asio;
  namespace ssl = boost::asio::ssl;

  SPDLOG_DEBUG("Query timesheets for all users");

  auto dnsLookupResultsOrError =
      dnsLookup(*ioContext_, appConfig_.credentials().server(), "https", yield);
  if (!dnsLookupResultsOrError) {
    SPDLOG_ERROR("Failed to dns lookup for url {}. Error: {}",
                 appConfig_.credentials().server(),
                 dnsLookupResultsOrError.error().message());
    return;
  }
  SPDLOG_INFO("Start date: {}",
              boost::gregorian::to_iso_extended_string(
                  appConfig_.options().dateStart()));
  SPDLOG_INFO(
      "End date: {}",
      boost::gregorian::to_iso_extended_string(appConfig_.options().dateEnd()));
  auto const requestStr = fmt::format(
      "/rest/timesheet-gadget/1.0/"
      "raw-timesheet.json?targetUser={}&startDate={}&endDate={}",
      "USERNAME",
      boost::gregorian::to_iso_extended_string(
          appConfig_.options().dateStart()),
      boost::gregorian::to_iso_extended_string(appConfig_.options().dateEnd()));
  http::request<http::empty_body> request{http::verb::get, requestStr, 10};

  request.set(http::field::authorization,
              "Basic " + base64Encode(
                             fmt::format("{}:{}",
                                         appConfig_.credentials().userName(),
                                         appConfig_.credentials().password())));
  std::stringstream ss;
  ss << request;
  SPDLOG_INFO("Request:\n{}", ss.str());
  auto const responseOrError = httpGet(*ioContext_,
                                       *sslContext_,
                                       dnsLookupResultsOrError.value(),
                                       request,
                                       std::chrono::seconds(10),
                                       yield);
  if (!responseOrError) {
    SPDLOG_ERROR("Failed to get data for user {}. Error: {}",
                 "rmalynovskyi",
                 responseOrError.error().message());
    return;
  }
  if (responseOrError.value().result() != http::status::ok) {
    SPDLOG_ERROR("Request has failed with result {}",
                 jwlrep::ToIntegral(responseOrError.value().result()));
    return;
  }
  auto const userTimeSheetOrError =
      createUserTimeSheetFromJson(responseOrError.value().body());
  if (!userTimeSheetOrError) {
    SPDLOG_ERROR("Failed to parse timesheet for user {}. Error: {}",
                 "rmalynovskyi",
                 responseOrError.error().message());
    return;
  }
  // move timesheet to heap to be able to move it to the queue
  auto const userTimeSheetPtr =
      std::make_unique<UserTimeSheet>(std::move(userTimeSheetOrError.value()));

  std::vector<std::reference_wrapper<UserTimeSheet>> timeSheets;
  timeSheets.reserve(1);
  timeSheets.push_back(*userTimeSheetPtr);

  createReportExcel(timeSheets, appConfig_.options());
}

} // namespace jwlrep