// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/Outcome.h>

#include <string>
#include <system_error>
#include <vector>

namespace jwlrep {

struct Credentials {
  std::string server;
  std::string userName;
  std::string password;
};

struct Options {
  std::int64_t weekNum;
  std::vector<std::string> users;
};

struct AppConfig {
  Credentials credentials;
  Options options;
};

Expected<AppConfig> processCmdArgs(int argc, char** argv);

} // namespace jwlrep