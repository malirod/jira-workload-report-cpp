// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/EnumUtil.h>
#include <jwlrep/GeneralError.h>

namespace jwlrep {

template <>
EnumStrings<GeneralError>::DataType EnumStrings<GeneralError>::data = {
    "Success",
    "Internal error",
    "Wrong startup parameter(s)",
    "Startup has failed",
    "Operation interrupted"};

const std::error_category& detail::ErrorCategory::get() {
  static ErrorCategory instance;
  return instance;
}

std::error_code make_error_code(GeneralError error) noexcept {
  return {ToIntegral(error), detail::ErrorCategory::get()};
}

const char* detail::ErrorCategory::name() const noexcept {
  return "GeneralError";
}

std::string detail::ErrorCategory::message(int error_value) const {
  return EnumToString(FromIntegral<GeneralError>(error_value));
}

} // namespace jwlrep