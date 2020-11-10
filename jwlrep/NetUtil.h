// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/Outcome.h>

#include <jwlrep/Logger.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/fiber/asio/yield.hpp>

#include <string_view>

namespace jwlrep {

Expected<boost::asio::ip::tcp::resolver::results_type> dnsLookup(
    boost::asio::io_context& ioContext,
    std::string_view const host,
    std::string_view const service,
    boost::fibers::asio::yield_t& yield);

Expected<boost::beast::http::response<boost::beast::http::string_body>> httpGet(
    boost::asio::io_context& ioContext,
    boost::asio::ssl::context& sslContext,
    boost::asio::ip::tcp::resolver::results_type const& address,
    boost::beast::http::request<boost::beast::http::empty_body> const& request,
    std::chrono::nanoseconds const timeout,
    boost::fibers::asio::yield_t& yield);

} // namespace jwlrep