// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/Logger.h>
#include <jwlrep/PathUtil.h>
#include <jwlrep/Version.h>

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <fstream>

namespace nlohmann {
template <>
struct adl_serializer<jwlrep::Credentials> {
  static jwlrep::Credentials from_json(json const& json) {
    return jwlrep::Credentials{json["server"].get<std::string>(),
                               json["userName"].get<std::string>(),
                               json["password"].get<std::string>()};
    ;
  }
};

template <>
struct adl_serializer<jwlrep::Options> {
  static jwlrep::Options from_json(json const& json) {
    return jwlrep::Options{
        boost::gregorian::from_string(json["dateStart"].get<std::string>()),
        boost::gregorian::from_string(json["dateEnd"].get<std::string>()),
        json["users"].get<std::vector<std::string>>()};
    ;
  }
};

template <>
struct adl_serializer<jwlrep::AppConfig> {
  static jwlrep::AppConfig from_json(json const& json) {
    return jwlrep::AppConfig{json["credentials"].get<jwlrep::Credentials>(),
                             json["options"].get<jwlrep::Options>()};
    ;
  }
};

} // namespace nlohmann

namespace {

bool isJsonValid(nlohmann::json const& jsonAppConfigJson) {
  auto const jsonSchema = R"(
  {
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "properties": {
        "credentials": {
            "type": "object",
            "properties": {"server": {"type": "string"},
                           "userName": {"type": "string"},
                           "userName": {"password": "string"}
                          },
            "required": [
                 "server",
                 "userName",
                 "password"
                 ]
        },
        "options": {
            "type": "object",
            "properties": {"dateStart": {"type": "string", "format": "date"},
                           "dateEnd": {"type": "string", "format": "date"},
                           "users": {"type": "array", "items": {"type": "string"}}
                          },
            "required": [
                 "dateStart",
                 "dateEnd",
                 "users"
                 ]
        }
    },
    "required": [
                 "credentials",
                 "options"
                 ]
  }
  )"_json;

  try {
    static nlohmann::json_schema::json_validator const validator(
        jsonSchema,
        nullptr,
        nlohmann::json_schema::default_string_format_check);
    // TODO: Extract error handler which will not use exceptions. Make util
    // class.
    validator.validate(jsonAppConfigJson);
  } catch (std::exception const& e) {
    SPDLOG_ERROR("App config validation has failed: {}", e.what());
    return false;
  }
  return true;
}

} // namespace

namespace jwlrep {

Expected<AppConfig> createAppConfigFromJson(
    std::string const& configFileJsonStr) {
  auto const configFileJson =
      nlohmann::json::parse(configFileJsonStr, nullptr, false, true);

  if (configFileJson.is_discarded()) {
    SPDLOG_ERROR("Failed to parse app config: json is not valid");
    return make_error_code(GeneralError::InvalidAppConfig);
  }

  if (!isJsonValid(configFileJson)) {
    return make_error_code(GeneralError::InvalidAppConfig);
  }

  return configFileJson.get<AppConfig>();
}

Expected<AppConfig> processCmdArgs(int argc, char** argv) {
  namespace po = boost::program_options;
  auto const printHelp = [](auto const& options) {
    std::stringstream sstream;
    sstream << options;
    SPDLOG_INFO(sstream.str());
  };
  auto const printVersion = []() { SPDLOG_INFO("Version: {}", getVersion()); };
  auto const printError = [](auto const& error) {
    SPDLOG_ERROR("Error: {}", error.what());
  };

  std::string configFilePath;
  po::options_description generalOptions("General options");
  generalOptions.add_options()("help,h", "Print help")(
      "version,v", "Print application version")(
      "config,c",
      po::value(&configFilePath)
          ->default_value(
              getExePath(argv[0]).replace_extension(".cfg").string()),
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

  if (vm.count("help") != 0u) {
    printHelp(generalOptions);
    return GeneralError::Interrupted;
  }

  if (vm.count("version") != 0u) {
    printVersion();
    return GeneralError::Interrupted;
  }

  try {
    std::ifstream configFileStream(configFilePath.c_str());
    if (!configFileStream) {
      SPDLOG_ERROR("Cannot open app configuration file: {}", configFilePath);
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

Credentials::Credentials(std::string const& server,
                         std::string const& userName,
                         std::string const& password)
    : server_(server), userName_(userName), password_(password) {
}

std::string const& Credentials::server() const {
  return server_;
}

std::string const& Credentials::userName() const {
  return userName_;
}

std::string const& Credentials::password() const {
  return password_;
}

Options::Options(boost::gregorian::date dateStart,
                 boost::gregorian::date dateEnd,
                 std::vector<std::string>&& users)
    : dateStart_(dateStart), dateEnd_(dateEnd), users_(std::move(users)) {
}

boost::gregorian::date const& Options::dateStart() const {
  return dateStart_;
}

boost::gregorian::date const& Options::dateEnd() const {
  return dateEnd_;
}

std::vector<std::string> const& Options::users() const {
  return users_;
}

AppConfig::AppConfig(Credentials&& credentials, Options&& options)
    : credentials_(credentials), options_(options) {
}

Credentials const& AppConfig::credentials() const {
  return credentials_;
}

Options const& AppConfig::options() const {
  return options_;
}

} // namespace jwlrep
