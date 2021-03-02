// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <cpr/cpr.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/Logger.h>
#include <jwlrep/NetUtil.h>

#include <magic_enum.hpp>
#include <system_error>

namespace jwlrep {

auto httpGet(std::string const& url, std::string userName, std::string password,
             std::chrono::milliseconds const& timeout)
    -> Expected<std::string> {
  auto responseFuture = cpr::GetCallback(
      [](cpr::Response const& response) {
        if (response.error.code != cpr::ErrorCode::OK) {
          LOG_DEBUG("HTTP get error. Code={}, message={}",
                    magic_enum::enum_name(response.error.code),
                    response.error.message);
          // return GeneralError::NetworkError;
        }
        return response.text;
      },
      cpr::Url{url},
      cpr::Authentication{std::move(userName), std::move(password)},
      cpr::Timeout{timeout});
  return responseFuture.get();
}

}  // namespace jwlrep