// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <boost/outcome.hpp>
#include <system_error>

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

namespace jwlrep {

template <typename T>
using Expected = outcome::result<T, std::error_code>;

} // namespace jwlrep