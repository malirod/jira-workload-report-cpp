// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

/*
 * Based on
 * http://codereview.stackexchange.com/questions/14309/conversion-between-enum-and-string-in-c-class-header
 */

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <type_traits>

namespace jwlrep {

template <typename E>
constexpr inline auto ToIntegral(E e) noexcept ->
    typename std::underlying_type<E>::type {
  return static_cast<typename std::underlying_type<E>::type>(e);
}

template <typename E, typename T>
constexpr inline auto FromIntegral(T value) noexcept -> typename std::enable_if<
    std::is_enum<E>::value && std::is_integral<T>::value, E>::type {
  return static_cast<E>(value);
}

/*
 * Reason for warning ignore.
 * In 3.9 clang introduced warning (-Wundefined-var-template) which
 * prevents main idea of enum util: split enum utils and filling Storage
 * in enum's cpp file via template specialization.
 */

#if defined(__clang__)
#pragma clang diagnostic push
#endif

#if defined(__clang__) && \
    (((__clang_major__ > 3) && (__clang_minor__ >= 9)) || __clang_major__ > 3)
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif

template <typename T>
struct Storage {  // NOLINT
  template <typename... ArgTypes>
  // cppcheck-suppress noExplicitConstructor
  Storage(ArgTypes... args)  // NOLINT
      : size(sizeof...(ArgTypes)) {
    static const char* static_data[sizeof...(ArgTypes)] = {args...};  // NOLINT
    data = static_data;
  }

  char const** data;
  int const size;
};

// Holds all strings.
// Each enumeration must declare its own specialization.
template <typename T>
struct EnumStrings {
  using DataType = Storage<T>;
  static DataType data;  // NOLINT
};

template <typename T>
struct EnumRefHolder {
  T& enum_value;  // NOLINT
  explicit EnumRefHolder(T& enum_value) : enum_value(enum_value) {}
};

template <typename T>
struct EnumConstRefHolder {
  T const& enum_value;  // NOLINT
  explicit EnumConstRefHolder(T const& enum_value) : enum_value(enum_value) {}
};

// Actual enum to string conversion
template <typename T>
auto operator<<(std::ostream& stream, EnumConstRefHolder<T> const& data)
    -> std::ostream& {
  auto const index = ToIntegral(data.enum_value);
  auto const data_size = EnumStrings<T>::data.size;
  if (index >= 0 && index < data_size) {
    stream << EnumStrings<T>::data.data[index];
  }
  return stream;
}

// Actual enum from string conversion
template <typename T>
auto operator>>(std::istream& stream, EnumRefHolder<T> const& data)
    -> std::istream& {
  std::string value;
  stream >> value;

  static auto begin = EnumStrings<T>::data.data;
  static auto end = EnumStrings<T>::data.data + EnumStrings<T>::data.size;

  auto find = std::find(begin, end, value);
  if (find != end) {
    data.enum_value = static_cast<T>(std::distance(begin, find));
  }

  return stream;
}

// This is the public interface:
// use the ability of function to deuce their template type without
// being explicitly told to create the correct type of enumRefHolder<T>
template <typename T>
auto EnumToStream(T const& e) -> EnumConstRefHolder<T> {
  return EnumConstRefHolder<T>(e);
}

template <typename T>
auto EnumFromStream(T& e) -> EnumRefHolder<T> {
  return EnumRefHolder<T>(e);
}

template <typename T>
auto EnumToChars(T const& e) -> char const* {
  auto const index = ToIntegral(e);
  auto const dataSize = EnumStrings<T>::data.size;
  if (index >= 0 && index < dataSize) {
    return EnumStrings<T>::data.data[index];
  }
  return "";
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

template <typename T>
auto EnumToString(T const& e) -> std::string {
  return std::string(EnumToChars<T>(e));
}

// http://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
struct EnumClassHash {
  template <typename T>
  auto operator()(T t) const -> std::size_t {
    return static_cast<std::size_t>(t);
  }
};

}  // namespace jwlrep
