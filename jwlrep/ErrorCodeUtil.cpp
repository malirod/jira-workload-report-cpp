// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/ErrorCodeUtil.h>
#include <jwlrep/GeneralError.h>

namespace jwlrep {

std::error_code toStd(boost::system::error_code const& errorCode) {
  return std::error_code(errorCode.value(), errorCode.category());
}

} // namespace jwlrep