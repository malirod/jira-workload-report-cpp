// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/EnumUtil.h>
#include <jwlrep/GeneralError.h>

namespace jwlrep {

template <>
EnumStrings<GeneralError>::DataType EnumStrings<GeneralError>::data = {
    "Success",           "Internal error",     "Wrong startup parameter(s)",
    "Invalid AppConfig", "Startup has failed", "Operation interrupted",
    "System error",      "Network error"};

auto detail::ErrorCategory::get() -> const std::error_category& {
  static ErrorCategory instance;
  return instance;
}

auto make_error_code(GeneralError error) noexcept -> std::error_code {
  return {ToIntegral(error), detail::ErrorCategory::get()};
}

auto detail::ErrorCategory::name() const noexcept -> const char* {
  return "GeneralError";
}

auto detail::ErrorCategory::message(int error_value) const -> std::string {
  return EnumToString(FromIntegral<GeneralError>(error_value));
}

}  // namespace jwlrep