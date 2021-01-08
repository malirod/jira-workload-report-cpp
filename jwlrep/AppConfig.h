// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/Outcome.h>

#include <boost/container/flat_map.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>
#include <vector>

namespace jwlrep {

class Credentials {
 public:
  Credentials(std::string server, std::string userName, std::string password);

  [[nodiscard]] auto server() const -> std::string const&;

  [[nodiscard]] auto userName() const -> std::string const&;

  [[nodiscard]] auto password() const -> std::string const&;

 private:
  std::string server_;

  std::string userName_;

  std::string password_;
};

class Options {
 public:
  Options(boost::gregorian::date dateStart, boost::gregorian::date dateEnd,
          std::vector<std::string>&& users, std::string defaultAssociation,
          boost::container::flat_map<std::string, std::string>&& associations);

  [[nodiscard]] auto dateStart() const -> boost::gregorian::date const&;

  [[nodiscard]] auto dateEnd() const -> boost::gregorian::date const&;

  [[nodiscard]] auto users() const -> std::vector<std::string> const&;

  [[nodiscard]] auto defaultAssociation() const -> std::string const&;

  [[nodiscard]] auto associations() const
      -> boost::container::flat_map<std::string, std::string> const&;

 private:
  boost::gregorian::date dateStart_;

  boost::gregorian::date dateEnd_;

  std::vector<std::string> users_;

  std::string defaultAssociation_;

  boost::container::flat_map<std::string, std::string> associations_;
};

class AppConfig {
 public:
  AppConfig(Credentials&& credentials, Options&& options);

  [[nodiscard]] auto credentials() const -> Credentials const&;

  [[nodiscard]] auto options() const -> Options const&;

 private:
  Credentials credentials_;

  Options options_;
};

auto createAppConfigFromJson(std::string const& configFileJsonStr)
    -> Expected<AppConfig>;

auto processCmdArgs(int argc, char** argv) -> Expected<AppConfig>;

}  // namespace jwlrep