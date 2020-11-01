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

namespace {

bool isJsonAppConfigValid(nlohmann::json const& jsonAppConfig) {
  auto const jsonSchema = R"(
  {
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Application Config",
    "properties": {
        "credentials": {
            "description": "Credentials to the Jira server",
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
            "description": "Report params",
            "type": "object",
            "properties": {"weekNum": {"type": "integer", "minimum": 1,
"maximum": 53}, "users": {"type": "array", "items": {"type": "string"}}
                          },
            "required": [
                 "weekNum",
                 "users"
                 ]
        }
    },
    "required": [
                 "credentials",
                 "options"
                 ],
    "type": "object"
  }
  )"_json;

  try {
    static nlohmann::json_schema::json_validator const validator(
        jsonSchema,
        nullptr,
        nlohmann::json_schema::default_string_format_check);
    validator.validate(jsonAppConfig);
  } catch (std::exception const& e) {
    SPDLOG_ERROR("App config validation has failed: {}", e.what());
    return false;
  }
  return true;
}

} // namespace

namespace jwlrep {

Expected<AppConfig> createAppConfig(std::string const& configFileText) {
  auto const configFileJson =
      nlohmann::json::parse(configFileText, nullptr, false, true);

  if (configFileJson.is_discarded()) {
    SPDLOG_ERROR("Failed to parse app config: json is not valid");
    return make_error_code(GeneralError::InvalidAppConfig);
  }

  if (!isJsonAppConfigValid(configFileJson)) {
    return make_error_code(GeneralError::InvalidAppConfig);
  }

  auto const server =
      configFileJson["credentials"]["server"].get<std::string>();

  auto credentials =
      Credentials{configFileJson["credentials"]["server"].get<std::string>(),
                  configFileJson["credentials"]["userName"].get<std::string>(),
                  configFileJson["credentials"]["password"].get<std::string>()};

  auto options = Options{
      configFileJson["options"]["weekNum"].get<std::int8_t>(),
      configFileJson["options"]["users"].get<std::vector<std::string>>()};

  return jwlrep::AppConfig{std::move(credentials), std::move(options)};
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
    return createAppConfig(configFileText);
  } catch (std::exception const& error) {
    printError(error);
    printHelp(generalOptions);
    return GeneralError::WrongStartupParams;
  }
}

} // namespace jwlrep
