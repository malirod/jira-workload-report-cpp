// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/ErrorCodeUtil.h>
#include <jwlrep/GeneralError.h>

namespace jwlrep {

auto toStd(boost::system::error_code const& errorCode) -> std::error_code {
  return std::error_code(errorCode.value(), errorCode.category());
}

}  // namespace jwlrep