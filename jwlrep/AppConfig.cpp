// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/JsonValidatorUtil.h>
#include <jwlrep/Logger.h>
#include <jwlrep/PathUtil.h>
#include <jwlrep/Version.h>

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

#include "boost/algorithm/string/case_conv.hpp"

namespace nlohmann {
template <>
struct adl_serializer<jwlrep::Credentials> {
  static auto from_json(json const& json) -> jwlrep::Credentials {
    return jwlrep::Credentials{json["serverUrl"].get<std::string>(),
                               json["userName"].get<std::string>(),
                               json["password"].get<std::string>()};
    ;
  }
};

template <>
struct adl_serializer<jwlrep::Options> {
  static auto from_json(json const& json) -> jwlrep::Options {
    boost::container::flat_map<std::string, std::string> associations;
    associations.reserve(json["associations"].size());

    for (auto const& [key, value] : json["associations"].items()) {
      // Make normalization of the Key
      associations.emplace(boost::algorithm::to_lower_copy(key), value);
    }

    return jwlrep::Options{
        boost::gregorian::from_string(json["dateStart"].get<std::string>()),
        boost::gregorian::from_string(json["dateEnd"].get<std::string>()),
        json["users"].get<std::vector<std::string>>(),
        json["defaultAssociation"].get<std::string>(), std::move(associations)};
    ;
  }
};

template <>
struct adl_serializer<jwlrep::AppConfig> {
  static auto from_json(json const& json) -> jwlrep::AppConfig {
    return jwlrep::AppConfig{json["credentials"].get<jwlrep::Credentials>(),
                             json["options"].get<jwlrep::Options>()};
    ;
  }
};

}  // namespace nlohmann

namespace {

auto isJsonValid(nlohmann::json const& jsonAppConfigJson) -> bool {
  auto const jsonSchema = R"(
  {
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "properties": {
        "credentials": {
            "type": "object",
            "additionalProperties": false,
            "properties": {"serverUrl": {"type": "string"},
                           "userName": {"type": "string"},
                           "password": {"password": "string"}
                          },
            "required": [
                 "serverUrl",
                 "userName",
                 "password"
                 ]
        },
        "options": {
            "type": "object",
            "additionalProperties": false,
            "properties": {"dateStart": {"type": "string", "format": "date"},
                           "dateEnd": {"type": "string", "format": "date"},
                           "users": {"type": "array", "items": {"type": "string"}},
                           "defaultAssociation": {"type": "string"},
                           "associations": {"type": "object", "additionalProperties": { "type": "string" }}
                          },
            "required": [
                 "dateStart",
                 "dateEnd",
                 "users",
                 "defaultAssociation",
                 "associations"
                 ]
        }
    },
    "required": [
                 "credentials",
                 "options"
                 ]
  }
  )"_json;

  static nlohmann::json_schema::json_validator const validator(
      jsonSchema, nullptr, nlohmann::json_schema::default_string_format_check);
  static jwlrep::JsonValidatorErrorHandler errorHandler;
  validator.validate(jsonAppConfigJson, errorHandler);
  if (errorHandler) {
    LOG_ERROR("App config validation has failed.");
    return false;
  }

  return true;
}

}  // namespace

namespace jwlrep {

auto createAppConfigFromJson(std::string const& configFileJsonStr)
    -> Expected<AppConfig> {
  auto const configFileJson =
      nlohmann::json::parse(configFileJsonStr, nullptr, false, true);

  if (configFileJson.is_discarded()) {
    LOG_ERROR("Failed to parse app config: json is not valid");
    return make_error_code(GeneralError::InvalidAppConfig);
  }

  if (!isJsonValid(configFileJson)) {
    return make_error_code(GeneralError::InvalidAppConfig);
  }

  return configFileJson.get<AppConfig>();
}

auto processCmdArgs(int argc, char** argv) -> Expected<AppConfig> {
  namespace po = boost::program_options;
  auto const printHelp = [](auto const& options) {
    std::stringstream sstream;
    sstream << options;
    LOG_INFO(sstream.str());
  };
  auto const printVersion = []() { LOG_INFO("Version: {}", kProjectVersion); };
  auto const printError = [](auto const& error) {
    LOG_ERROR("Error: {}", error.what());
  };

  // Assume that in the first arg exe path is stored. On some platforms it's not
  // true;
  assert(argc > 0);
  assert(argv[0] != nullptr);
  auto const defaultConfigPath =
      getExePath(argv[0]).replace_extension(".cfg").string();

  std::string configFilePath;
  po::options_description generalOptions("General options");
  generalOptions.add_options()("help,h", "Print help")(
      "version,v", "Print application version")(
      "config,c", po::value(&configFilePath)->default_value(defaultConfigPath),
      "Configuration file path");

  po::variables_map vm;
  try {
    po::parsed_options parsed_options = po::command_line_parser(argc, argv)
                                            .options(generalOptions)
                                            .allow_unregistered()
                                            .run();

    po::store(parsed_options, vm);
    po::notify(vm);
  } catch (po::error const& error) {
    printError(error);
    printHelp(generalOptions);
    return GeneralError::WrongStartupParams;
  }

  if (vm.count("help") != 0) {
    printHelp(generalOptions);
    return GeneralError::Interrupted;
  }

  if (vm.count("version") != 0) {
    printVersion();
    return GeneralError::Interrupted;
  }

  try {
    std::ifstream configFileStream(configFilePath.c_str());
    if (!configFileStream) {
      LOG_ERROR("Cannot open app configuration file: {}", configFilePath);
      return GeneralError::Interrupted;
    }
    auto const configFileText =
        std::string{std::istreambuf_iterator{configFileStream}, {}};
    return createAppConfigFromJson(configFileText);
  } catch (std::exception const& error) {
    printError(error);
    printHelp(generalOptions);
    return GeneralError::WrongStartupParams;
  }
}

Credentials::Credentials(std::string serverUrl, std::string userName,
                         std::string password)
    : serverUrl_(std::move(serverUrl)),
      userName_(std::move(userName)),
      password_(std::move(password)) {}

auto Credentials::serverUrl() const -> std::string const& { return serverUrl_; }

auto Credentials::userName() const -> std::string const& { return userName_; }

auto Credentials::password() const -> std::string const& { return password_; }

Options::Options(
    boost::gregorian::date dateStart, boost::gregorian::date dateEnd,
    std::vector<std::string>&& users, std::string defaultAssociation,
    boost::container::flat_map<std::string, std::string>&& associations)
    : dateStart_(dateStart),
      dateEnd_(dateEnd),
      users_(std::move(users)),
      defaultAssociation_(std::move(defaultAssociation)),
      associations_(std::move(associations)) {}

auto Options::dateStart() const -> boost::gregorian::date const& {
  return dateStart_;
}

auto Options::dateEnd() const -> boost::gregorian::date const& {
  return dateEnd_;
}

auto Options::users() const -> std::vector<std::string> const& {
  return users_;
}

auto Options::defaultAssociation() const -> std::string const& {
  return defaultAssociation_;
}

auto Options::associations() const
    -> boost::container::flat_map<std::string, std::string> const& {
  return associations_;
}

AppConfig::AppConfig(Credentials&& credentials, Options&& options)
    : credentials_(credentials), options_(options) {}

auto AppConfig::credentials() const -> Credentials const& {
  return credentials_;
}

auto AppConfig::options() const -> Options const& { return options_; }

}  // namespace jwlrep
