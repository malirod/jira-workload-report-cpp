// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/Outcome.h>

#include <boost/date_time/gregorian/gregorian.hpp>

#include <string>
#include <vector>

namespace jwlrep {

class Credentials {
 public:
  Credentials(std::string const& server,
              std::string const& userName,
              std::string const& password);

  std::string const& server() const;

  std::string const& userName() const;

  std::string const& password() const;

 private:
  std::string server_;

  std::string userName_;

  std::string password_;
};

class Options {
 public:
  Options(boost::gregorian::date dateStart,
          boost::gregorian::date dateEnd,
          std::vector<std::string>&& users);

  boost::gregorian::date const& dateStart() const;

  boost::gregorian::date const& dateEnd() const;

  std::vector<std::string> const& users() const;

 private:
  boost::gregorian::date dateStart_;

  boost::gregorian::date dateEnd_;

  std::vector<std::string> users_;
};

class AppConfig {
 public:
  AppConfig(Credentials&& credentials, Options&& options);

  Credentials const& credentials() const;

  Options const& options() const;

 private:
  Credentials credentials_;

  Options options_;
};

Expected<AppConfig> createAppConfigFromJson(
    std::string const& configFileJsonStr);

Expected<AppConfig> processCmdArgs(int argc, char** argv);

} // namespace jwlrep