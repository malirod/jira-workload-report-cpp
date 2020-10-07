// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/PathUtil.h>
#include <jwlrep/Version.h>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <folly/json.h>
#include <folly/logging/xlog.h>

#include <fstream>

namespace {

jwlrep::AppConfig createAppConfig(folly::dynamic const& dynAppConfig) {
  jwlrep::AppConfig result;
  result.credentials.server = dynAppConfig["credentials"]["server"].asString();
  result.credentials.userName =
      dynAppConfig["credentials"]["userName"].asString();
  result.credentials.password =
      dynAppConfig["credentials"]["password"].asString();
  result.options.weekNum = dynAppConfig["options"]["weekNum"].asInt();
  result.options.users.reserve(dynAppConfig["options"]["users"].size());
  for (auto const& user : dynAppConfig["options"]["users"]) {
    result.options.users.push_back(user.asString());
  }
  return result;
}

} // namespace

namespace jwlrep {

folly::Expected<AppConfig, std::error_code> processCmdArgs(int argc,
                                                           char** argv) {
  namespace po = boost::program_options;

  auto const printHelp = [](auto const& options) { XLOG(INFO) << options; };
  auto const printVersion = []() { XLOGF(INFO, "Version: {}", getVersion()); };
  auto const printError = [](auto const& error) {
    XLOGF(ERR, "Error: {}", error.what());
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
    return folly::makeUnexpected(
        make_error_code(GeneralError::WrongStartupParams));
  }

  if (vm.count("help") != 0u) {
    printHelp(generalOptions);
    return folly::makeUnexpected(make_error_code(GeneralError::Interrupted));
  }

  if (vm.count("version") != 0u) {
    printVersion();
    return folly::makeUnexpected(make_error_code(GeneralError::Interrupted));
  }

  try {
    std::ifstream configFileStream(configFilePath.c_str());
    if (!configFileStream) {
      XLOGF(ERR, "Cannot open app configuration file: {}", configFilePath);
      return folly::makeUnexpected(make_error_code(GeneralError::Interrupted));
    }
    auto const configFileString =
        std::string{(std::istreambuf_iterator<char>(configFileStream)),
                    std::istreambuf_iterator<char>()};
    auto const parsedConfig = folly::parseJson(std::move(configFileString));
    return createAppConfig(parsedConfig);
  } catch (std::exception const& error) {
    printError(error);
    printHelp(generalOptions);
    return folly::makeUnexpected(
        make_error_code(GeneralError::WrongStartupParams));
  }
}

} // namespace jwlrep