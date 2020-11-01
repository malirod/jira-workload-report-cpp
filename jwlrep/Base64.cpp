// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Base64.h>

#include <boost/beast/core/detail/base64.hpp>

namespace {

std::string base64Encode(std::uint8_t const* data, std::size_t length) {
  std::string result;
  result.resize(boost::beast::detail::base64::encoded_size(length));
  result.resize(boost::beast::detail::base64::encode(&result[0], data, length));
  return result;
}

} // namespace

namespace jwlrep {

std::string base64Encode(std::string_view dataView) {
  return ::base64Encode(reinterpret_cast<std::uint8_t const*>(dataView.data()),
                        dataView.size());
}

} // namespace jwlrep