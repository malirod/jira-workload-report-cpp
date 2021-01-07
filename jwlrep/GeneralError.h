// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <string>
#include <system_error>

namespace jwlrep {

/**
 * Enum which describes general Engine errors.
 */
enum class GeneralError {
  Success,
  InternalError,
  WrongStartupParams,
  InvalidAppConfig,
  StartupFailed,
  Interrupted,
  SystemError,
  NetworkError
};

namespace detail {

/**
 * Error category for general errors.
 */
class ErrorCategory : public std::error_category {
 public:
  /**
   * Gets category name.
   * @return Name of the category.
   */
  [[nodiscard]] auto name() const noexcept -> const char* override;

  /**
   * Convert error code to corresponding message string.
   * @param error_value Error code
   * @return Error massage
   */
  [[nodiscard]] auto message(int error_value) const -> std::string override;

  /**
   * Allows to get access to single instance of this category.
   * @return This Category.
   */
  static auto get() -> const std::error_category&;

 protected:
  ErrorCategory() = default;
};

}  // namespace detail

// Overload the global make_error_code() free function with our
// custom enum. It will be found via ADL by the compiler if needed.
auto make_error_code(GeneralError error) noexcept -> std::error_code;

}  // namespace jwlrep

namespace std {
// Tell the C++ 11 STL metaprogramming that enum ConversionErrc
// is registered with the standard error code system
template <>
struct is_error_code_enum<jwlrep::GeneralError> : std::true_type {};
}  // namespace std