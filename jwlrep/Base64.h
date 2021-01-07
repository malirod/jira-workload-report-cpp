// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <string_view>

namespace jwlrep {

std::string base64Encode(std::string_view dataView);

}  // namespace jwlrep