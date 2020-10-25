// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/Outcome.h>

#include <string>
#include <vector>

namespace jwlrep {

struct Credentials {
  std::string const server;
  std::string const userName;
  std::string const password;
};

struct Options {
  std::int8_t const weekNum;
  std::vector<std::string> const users;
};

struct AppConfig {
  Credentials const credentials;
  Options const options;
};

Expected<AppConfig> createAppConfig(std::string const& configFileText);

Expected<AppConfig> processCmdArgs(int argc, char** argv);

} // namespace jwlrep