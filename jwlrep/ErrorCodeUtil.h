// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <boost/system/error_code.hpp>
#include <system_error>

namespace jwlrep {

auto toStd(boost::system::error_code const& errorCode) -> std::error_code;

}  // namespace jwlrep