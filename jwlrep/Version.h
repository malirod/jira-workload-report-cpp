// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <string>

namespace jwlrep {

const unsigned int kMajor = 1;
const unsigned int kMinor = 0;
const unsigned int kPatch = 0;

/**
 * Get current version.
 * @return version as string
 */
std::string getVersion();

} // namespace jwlrep