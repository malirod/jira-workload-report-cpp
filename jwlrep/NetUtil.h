// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/Outcome.h>

#include <chrono>

namespace jwlrep {

auto httpGet(std::string const& url, std::string userName, std::string password,
             std::chrono::milliseconds const& timeout) -> Expected<std::string>;

}  // namespace jwlrep