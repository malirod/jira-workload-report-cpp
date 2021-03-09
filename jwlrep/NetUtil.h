// SPDX-License-Identifier: MIT

// Copyright (C) 2021 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/Logger.h>
#include <jwlrep/Outcome.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <string_view>

#include "boost/asio/any_io_executor.hpp"

namespace jwlrep {

Expected<boost::asio::ip::tcp::resolver::results_type> dnsLookup(
    boost::asio::any_io_executor& ioExecutor, std::string_view const host,
    std::string_view const service, boost::asio::yield_context& yield);

Expected<boost::beast::http::response<boost::beast::http::string_body>> httpGet(
    boost::asio::any_io_executor& ioExecutor,
    boost::asio::ssl::context& sslContext,
    boost::asio::ip::tcp::resolver::results_type const& address,
    boost::beast::http::request<boost::beast::http::empty_body> const& request,
    std::chrono::nanoseconds const timeout, boost::asio::yield_context& yield);

}  // namespace jwlrep