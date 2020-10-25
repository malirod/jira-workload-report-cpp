// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <boost/asio/io_context.hpp>

#include <string>

namespace jwlrep {

auto resolveAsync(boost::asio::io_context& ioContext, std::string const& host);

} // namespace jwlrep