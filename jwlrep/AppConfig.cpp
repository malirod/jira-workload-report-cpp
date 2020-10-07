// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/Logger.h>
#include <jwlrep/PathUtil.h>
#include <jwlrep/Version.h>

#include <nlohmann/json.hpp>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <fstream>

namespace jwlrep {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(jwlrep::Credentials,
                                   server,
                                   userName,
                                   password)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(jwlrep::Options, weekNum, users)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(jwlrep::AppConfig, credentials, options)

} // namespace jwlrep

namespace {

jwlrep::AppConfig createAppConfig(std::ifstream& configFileStream) {
  assert(configFileStream);
  nlohmann::json configFileJson;
  configFileStream >> configFileJson;

  return configFileJson.get<jwlrep::AppConfig>();
}

} // namespace
namespace jwlrep {

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
    return createAppConfig(configFileStream);
  } catch (std::exception const& error) {
    printError(error);
    printHelp(generalOptions);
    return GeneralError::WrongStartupParams;
  }
}

} // namespace jwlrep
