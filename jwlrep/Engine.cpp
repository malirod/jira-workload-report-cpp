// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/Base64.h>
#include <jwlrep/Engine.h>
#include <jwlrep/ExcelReport.h>
#include <jwlrep/IEngineEventHandler.h>
#include <jwlrep/Logger.h>
#include <jwlrep/NetUtil.h>
#include <jwlrep/RootCertificates.h>
#include <jwlrep/ScopeGuard.h>
#include <jwlrep/Worklog.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/fiber/asio/round_robin.hpp>
#include <boost/fiber/asio/yield.hpp>
#include <cassert>
#include <magic_enum.hpp>
#include <utility>

namespace jwlrep {

Engine::Engine(std::shared_ptr<boost::asio::io_context> ioContext,
               IEngineEventHandler& engineEventHandler,
               AppConfig const& appConfig)
    : ioContext_(std::move(ioContext)),
      engineEventHandler_(engineEventHandler),
      appConfig_(appConfig) {
  LOG_DEBUG("Engine has been created.");
  assert(ioContext_);
  boost::fibers::use_scheduling_algorithm<boost::fibers::asio::round_robin>(
      ioContext_);

  sslContext_ = std::make_unique<boost::asio::ssl::context>(
      boost::asio::ssl::context::tlsv12_client);

  loadRootCertificates(*sslContext_);

  sslContext_->set_verify_mode(boost::asio::ssl::verify_peer);
}

void Engine::start() {
  LOG_DEBUG("Starting engine");

  boost::fibers::fiber([this]() {
    LOG_DEBUG("Launched main fiber");

    ScopeGuard const guard{[&]() {
      LOG_DEBUG("Finished main fiber");
      stop();
    }};

    try {
      boost::asio::post(*ioContext_,
                        [this]() { engineEventHandler_.onEngineStarted(); });

      auto const timeSheetsOrError = loadTimesheets();

      if (!timeSheetsOrError) {
        LOG_ERROR("Failed to load timesheets: {}",
                  timeSheetsOrError.error().message());
        return;
      }

      generateTimesheetsXSLTReport(timeSheetsOrError.value());

    } catch (std::exception const& e) {
      LOG_ERROR("Got exception in main fiber: {}", e.what());
    }
  }).detach();

  LOG_DEBUG("Engine has been launched.");
}

void Engine::stop() {
  LOG_DEBUG("Stopping engine");

  boost::asio::post(*ioContext_,
                    [this]() { engineEventHandler_.onEngineStopped(); });
}

auto Engine::loadTimesheets() -> Expected<TimeSheets> {
  namespace http = boost::beast::http;
  auto& yield = boost::fibers::asio::this_yield();

  auto makeHttpRequest = [&options = appConfig_.options(),
                          &credentials =
                              appConfig_.credentials()](auto const& userName) {
    auto const requestStr = fmt::format(
        "/rest/timesheet-gadget/1.0/"
        "raw-timesheet.json?targetUser={}&startDate={}&endDate={}",
        userName, boost::gregorian::to_iso_extended_string(options.dateStart()),
        boost::gregorian::to_iso_extended_string(options.dateEnd()));
    auto const kHTTPVersion = 10;
    http::request<http::empty_body> request{http::verb::get, requestStr,
                                            kHTTPVersion};

    request.set(
        http::field::authorization,
        "Basic " + base64Encode(fmt::format("{}:{}", credentials.userName(),
                                            credentials.password())));
    return request;
  };

  LOG_DEBUG(
      "Query timesheets for all users for time period: {} - {}",
      boost::gregorian::to_iso_extended_string(
          appConfig_.options().dateStart()),
      boost::gregorian::to_iso_extended_string(appConfig_.options().dateEnd()));

  auto dnsLookupResultsOrError =
      dnsLookup(*ioContext_, appConfig_.credentials().serverUrl().host(),
                appConfig_.credentials().serverUrl().scheme(), yield);
  if (!dnsLookupResultsOrError) {
    LOG_ERROR("Failed to make dns lookup for url {}://{}. Error: {}",
              appConfig_.credentials().serverUrl().scheme(),
              appConfig_.credentials().serverUrl().host(),
              dnsLookupResultsOrError.error().message());
    return dnsLookupResultsOrError.error();
  }

  TimeSheets timeSheets;
  timeSheets.reserve(appConfig_.options().users().size());

  // It has to be shared ptr otherwise there will be the crash. Due to
  // cooperative mode outer function will exit (and barrier will be destroyed)
  // before last sub-fiber will exit
  auto barrier = std::make_shared<boost::fibers::barrier>(
      appConfig_.options().users().size() + 1);
  for (auto const& user : appConfig_.options().users()) {
    LOG_DEBUG("Start getting data for the user {}", user);
    boost::fibers::fiber([barrier, &makeHttpRequest, &dnsLookupResultsOrError,
                          &user, &yield, &timeSheets, this]() {
      auto request = makeHttpRequest(user);

      auto const responseOrError =
          httpGet(*ioContext_, *sslContext_, dnsLookupResultsOrError.value(),
                  request, std::chrono::seconds(10), yield);
      if (!responseOrError) {
        LOG_ERROR("Failed to get data for user {}. Error: {}", user,
                  responseOrError.error().message());
        return;
      }
      if (responseOrError.value().result() != http::status::ok) {
        LOG_ERROR("Request has failed with result {}",
                  magic_enum::enum_integer(responseOrError.value().result()));
        return;
      }
      auto userTimeSheetOrError =
          createUserTimeSheetFromJson(responseOrError.value().body());
      if (!userTimeSheetOrError) {
        LOG_ERROR("Failed to parse timesheet for user {}. Error: {}", user,
                  responseOrError.error().message());
        return;
      }

      timeSheets.emplace_back(std::move(userTimeSheetOrError.value()));

      LOG_DEBUG("Got data for the user {}", user);

      barrier->wait();
    }).detach();
  }

  barrier->wait();
  LOG_DEBUG("Got all timesheets");
  return timeSheets;
}

void Engine::generateTimesheetsXSLTReport(TimeSheets const& timeSheets) {
  LOG_DEBUG("Generating report");
  createReportExcel(timeSheets, appConfig_.options());
}

}  // namespace jwlrep