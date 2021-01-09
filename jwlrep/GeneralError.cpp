// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/GeneralError.h>

#include <magic_enum.hpp>

namespace jwlrep {

auto detail::ErrorCategory::get() -> const std::error_category& {
  static ErrorCategory instance;
  return instance;
}

auto make_error_code(GeneralError error) noexcept -> std::error_code {
  return {magic_enum::enum_integer(error), detail::ErrorCategory::get()};
}

auto detail::ErrorCategory::name() const noexcept -> const char* {
  return "GeneralError";
}

auto detail::ErrorCategory::message(int errorValue) const -> std::string {
  auto const enumValueOrError = magic_enum::enum_cast<GeneralError>(errorValue);
  if (!enumValueOrError.has_value()) {
    return "Unknown error code " + std::to_string(errorValue);
  }
  auto const view = magic_enum::enum_name(enumValueOrError.value());
  return {view.begin(), view.end()};
}

}  // namespace jwlrep