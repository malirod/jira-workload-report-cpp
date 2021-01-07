// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fmt/format.h>
#include <jwlrep/Version.h>

namespace jwlrep {

const unsigned int kMajor = 1;
const unsigned int kMinor = 0;
const unsigned int kPatch = 0;

auto getVersion() -> std::string {
  return fmt::format("{}.{}.{}", kMajor, kMinor, kPatch);
}

}  // namespace jwlrep