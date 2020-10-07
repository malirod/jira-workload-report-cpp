// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Version.h>

#include <fmt/format.h>

namespace jwlrep {

std::string getVersion() {
  return fmt::format("{}.{}.{}", kMajor, kMinor, kPatch);
}

} // namespace jwlrep