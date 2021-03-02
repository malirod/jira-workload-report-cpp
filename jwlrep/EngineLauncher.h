// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/AppConfig.h>
#include <jwlrep/GeneralError.h>

namespace jwlrep {

/**
 * Encapsulates logic of initialization of Engine and startup.
 */
class EngineLauncher {
 public:
  explicit EngineLauncher(AppConfig appConfig);

  EngineLauncher(EngineLauncher const&) = delete;
  EngineLauncher(EngineLauncher const&&) = delete;
  auto operator=(EngineLauncher const&) -> EngineLauncher& = delete;
  auto operator=(EngineLauncher const&&) -> EngineLauncher& = delete;

  auto run() -> std::error_code;

 private:
  AppConfig const appConfig_;
};

}  // namespace jwlrep
