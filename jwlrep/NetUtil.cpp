// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/NetUtil.h>

#include <jwlrep/Logger.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

namespace jwlrep {

auto resolveAsync(boost::asio::io_context& ioContext, std::string const& host) {
  boost::beast::error_code errorCode;
  boost::asio::ip::tcp::resolver resolver(ioContext);
  boost::beast::tcp_stream stream(ioContext);

  // Look up the domain name
  /*
  auto const results = resolver.async_resolve(host, yield[errorCode]);
  if (errorCode) {
    SPDLOG_ERROR(
        "Failed to resolve host {}. Error: {}", host, errorCode.message());
    // return fail(ec, "resolve");
  }
  return results;
  */
}

} // namespace jwlrep