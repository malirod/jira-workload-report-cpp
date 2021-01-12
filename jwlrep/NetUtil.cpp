// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/ErrorCodeUtil.h>
#include <jwlrep/Logger.h>
#include <jwlrep/NetUtil.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/fiber/asio/yield.hpp>
#include <system_error>

namespace jwlrep {

auto dnsLookup(boost::asio::io_context& ioContext, std::string_view const host,
               std::string_view const service,
               boost::fibers::asio::yield_t& yield)
    -> Expected<boost::asio::ip::tcp::resolver::results_type> {
  boost::beast::error_code errorCode;
  boost::asio::ip::tcp::resolver resolver(ioContext);
  auto dnsLookupResults =
      resolver.async_resolve(host, service, yield[errorCode]);
  if (errorCode) {
    LOG_ERROR("Failed to resolve {}:{}. Error: {}", host, service,
              errorCode.message());
    return toStd(errorCode);
  }
  return dnsLookupResults;
}

auto httpGet(
    boost::asio::io_context& ioContext, boost::asio::ssl::context& sslContext,
    boost::asio::ip::tcp::resolver::results_type const& address,
    boost::beast::http::request<boost::beast::http::empty_body> const& request,
    std::chrono::nanoseconds const timeout, boost::fibers::asio::yield_t& yield)
    -> Expected<boost::beast::http::response<boost::beast::http::string_body>> {
  namespace http = boost::beast::http;
  namespace beast = boost::beast;
  namespace net = boost::asio;
  namespace ssl = boost::asio::ssl;

  beast::error_code errorCode;
  beast::ssl_stream<beast::tcp_stream> stream(ioContext, sslContext);

  beast::get_lowest_layer(stream).expires_after(timeout);

  beast::get_lowest_layer(stream).async_connect(address, yield[errorCode]);
  if (errorCode) {
    LOG_ERROR("Failed to connet. Error: {}", errorCode.message());
    return toStd(errorCode);
  }

  beast::get_lowest_layer(stream).expires_after(timeout);
  stream.async_handshake(ssl::stream_base::client, yield[errorCode]);
  if (errorCode) {
    LOG_ERROR("Failed to make handshake. Error: {}", errorCode.message());
    return toStd(errorCode);
  }

  beast::get_lowest_layer(stream).expires_after(timeout);
  http::async_write(stream, request, yield[errorCode]);
  if (errorCode) {
    LOG_ERROR("Failed to write. Error: {}", errorCode.message());
    return toStd(errorCode);
  }

  beast::flat_buffer buffer;
  http::response<http::string_body> response;
  http::async_read(stream, buffer, response, yield[errorCode]);
  if (errorCode) {
    LOG_ERROR("Failed to read. Error: {}", errorCode.message());
    return toStd(errorCode);
  }

  beast::get_lowest_layer(stream).expires_after(timeout);
  stream.async_shutdown(yield[errorCode]);
  if (errorCode && errorCode != net::error::eof) {
    LOG_ERROR("Failed to shutdown. Error: {}", errorCode.message());
    return toStd(errorCode);
  }

  return response;
}

}  // namespace jwlrep