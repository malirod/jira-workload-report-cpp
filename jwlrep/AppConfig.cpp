// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/Logger.h>
#include <jwlrep/PathUtil.h>
#include <jwlrep/Version.h>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

namespace nlohmann {
template <>
struct adl_serializer<jwlrep::Credentials> {
  static auto from_json(json const& json) -> jwlrep::Credentials {
    return jwlrep::Credentials{json["server"].get<std::string>(),
                               json["userName"].get<std::string>(),
                               json["password"].get<std::string>()};
    ;
  }
};

template <>
struct adl_serializer<jwlrep::Options> {
  static auto from_json(json const& json) -> jwlrep::Options {
    return jwlrep::Options{
        boost::gregorian::from_string(json["dateStart"].get<std::string>()),
        boost::gregorian::from_string(json["dateEnd"].get<std::string>()),
        json["users"].get<std::vector<std::string>>()};
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
        jsonSchema, nullptr,
        nlohmann::json_schema::default_string_format_check);
    // TODO(malirod): Extract error handler which will not use exceptions. Make
    // util class.
    validator.validate(jsonAppConfigJson);
  } catch (std::exception const& e) {
    LOG_ERROR("App config validation has failed: {}", e.what());
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
  auto const printVersion = []() { LOG_INFO("Version: {}", getVersion()); };
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

Credentials::Credentials(std::string server, std::string userName,
                         std::string password)
    : server_(std::move(server)),
      userName_(std::move(userName)),
      password_(std::move(password)) {}

auto Credentials::server() const -> std::string const& { return server_; }

auto Credentials::userName() const -> std::string const& { return userName_; }

auto Credentials::password() const -> std::string const& { return password_; }

Options::Options(boost::gregorian::date dateStart,
                 boost::gregorian::date dateEnd,
                 std::vector<std::string>&& users)
    : dateStart_(dateStart), dateEnd_(dateEnd), users_(std::move(users)) {}

auto Options::dateStart() const -> boost::gregorian::date const& {
  return dateStart_;
}

auto Options::dateEnd() const -> boost::gregorian::date const& {
  return dateEnd_;
}

auto Options::users() const -> std::vector<std::string> const& {
  return users_;
}

AppConfig::AppConfig(Credentials&& credentials, Options&& options)
    : credentials_(credentials), options_(options) {}

auto AppConfig::credentials() const -> Credentials const& {
  return credentials_;
}

auto AppConfig::options() const -> Options const& { return options_; }

}  // namespace jwlrep
