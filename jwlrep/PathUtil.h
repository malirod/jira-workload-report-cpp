// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <filesystem>

namespace jwlrep {

auto getExePath(char const* argv0) -> std::filesystem::path;

}  // namespace jwlrep