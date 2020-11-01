// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Engine.h>

#include <jwlrep/AppConfig.h>
#include <jwlrep/Base64.h>
#include <jwlrep/EnumUtil.h>
#include <jwlrep/IEngineEventHandler.h>
#include <jwlrep/Logger.h>
#include <jwlrep/RootCertificates.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

#include <boost/fiber/asio/round_robin.hpp>

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

  // boost::asio::post(
  //    *ioContext_, [this]() { auto const timeTimesheets = queryTimesheets();
  //    });

  sslContext_ = std::make_unique<boost::asio::ssl::context>(
      boost::asio::ssl::context::tlsv12_client);

  // This holds the root certificate used for verification
  loadRootCertificates(*sslContext_);

  // Verify the remote server's certificate
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

  // channel_->push(1);
  // channel_->push(2);
  // channel_->push(3);
  // channel_->close();

  return;
}

void Engine::stop() {
  SPDLOG_DEBUG("Stopping engine");
  assert(initiated_);

  /*
  if (ioContext_->stopped()) {
    SPDLOG_DEBUG("Already stopped. Skip.");
    return;
  }*/
  if (channel_->is_closed()) {
    SPDLOG_DEBUG("Already stopped. Skip.");
    return;
  }
  channel_->push(100);
  channel_->close();

  // boost::asio::post(*ioContext_,
  //                  [this]() { engineEventHandler_.onEngineStopped(); });

  // ioContext_->stop();

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
  auto& yield = boost::fibers::asio::yield;
  namespace beast = boost::beast;
  namespace net = boost::asio;
  namespace ssl = boost::asio::ssl;

  SPDLOG_DEBUG("Query timesheets for all users");
  // auto const endpoints = resolveAsync(appConfig_.credentials.server);
  beast::error_code errorCode;
  tcp::resolver resolver(*ioContext_);
  // beast::tcp_stream stream(*ioContext_);
  beast::ssl_stream<beast::tcp_stream> stream(*ioContext_, *sslContext_);

  // Set SNI Hostname (many hosts need this to handshake successfully)
  if (!SSL_set_tlsext_host_name(stream.native_handle(),
                                appConfig_.credentials.server.c_str())) {
    errorCode.assign(static_cast<int>(::ERR_get_error()),
                     net::error::get_ssl_category());
    SPDLOG_ERROR("Failed to set tlsext host name. Host {}, error: {}",
                 appConfig_.credentials.server,
                 errorCode.message());
    return;
  }

  auto const timeout = std::chrono::seconds(10);

  SPDLOG_DEBUG("Resolving host");

  auto const dnsLookupResults = resolver.async_resolve(
      appConfig_.credentials.server, "https", yield[errorCode]);
  SPDLOG_DEBUG("Resolved");
  if (errorCode) {
    SPDLOG_ERROR("Failed to resolve host {}. Error: {}",
                 appConfig_.credentials.server,
                 errorCode.message());
    return;
  }
  // SPDLOG_DEBUG("Host: {}", *results. address().to_string());
  tcp::endpoint endpoint = *dnsLookupResults;
  SPDLOG_DEBUG("Host: {}", endpoint.address().to_string());

  // Set the timeout.
  beast::get_lowest_layer(stream).expires_after(timeout);

  // Make the connection on the IP address we get from a lookup
  beast::get_lowest_layer(stream).async_connect(dnsLookupResults,
                                                yield[errorCode]);
  if (errorCode) {
    SPDLOG_ERROR("Failed to connet. Error: {}", errorCode.message());
    return;
  }

  beast::get_lowest_layer(stream).expires_after(timeout);
  stream.async_handshake(ssl::stream_base::client, yield[errorCode]);
  if (errorCode) {
    SPDLOG_ERROR("Failed to make handshake. Error: {}", errorCode.message());
    return;
  }

  auto const requestStr = fmt::format(
      "/rest/timesheet-gadget/1.0/"
      "raw-timesheet.json?targetUser={}&startDate={}&endDate={}",
      base64Encode("rmalynovskyi"),
      base64Encode("2020-10-01"),
      base64Encode("2020-10-31"));
  http::request<http::empty_body> request{http::verb::get, requestStr, 10};

  request.set(
      http::field::authorization,
      "Basic " + base64Encode(fmt::format("{}:{}",
                                          appConfig_.credentials.userName,
                                          appConfig_.credentials.password)));
  // std::stringstream ssReq;
  // ssReq << request;
  // SPDLOG_DEBUG("Request:\n{}", ssReq.str());

  beast::get_lowest_layer(stream).expires_after(timeout);
  http::async_write(stream, request, yield[errorCode]);
  if (errorCode) {
    SPDLOG_ERROR("Failed to write. Error: {}", errorCode.message());
    return;
  }

  beast::flat_buffer buffer;
  http::response<http::string_body> response;
  http::async_read(stream, buffer, response, yield[errorCode]);
  if (errorCode) {
    SPDLOG_ERROR("Failed to read. Error: {}", errorCode.message());
    return;
  }
  if (response.result() != http::status::ok) {
    SPDLOG_ERROR("Http request has failed with status {}. Body:\n{}",
                 ToIntegral(response.result()),
                 response.body().data());
    return;
  }
  // std::stringstream ssRes;
  // ssRes << response;
  // SPDLOG_DEBUG("Response:\n{}", ssRes.str());
  // SPDLOG_DEBUG("Response:\n{}", response.body().data());

  beast::get_lowest_layer(stream).expires_after(timeout);
  stream.async_shutdown(yield[errorCode]);
  if (errorCode && errorCode != net::error::eof) {
    SPDLOG_ERROR("Failed to shutdown. Error: {}", errorCode.message());
    return;
  }
}

} // namespace jwlrep