// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/EngineLauncher.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/PathUtil.h>
#include <jwlrep/Version.h>

#include <folly/init/Init.h>
#include <folly/logging/Init.h>
#include <folly/logging/LogConfigParser.h>
#include <folly/logging/LoggerDB.h>
#include <folly/logging/xlog.h>

#include <cassert>
#include <fstream>

FOLLY_INIT_LOGGING_CONFIG("INFO; default=stream:stream=stdout,async=false");

std::error_code setupLogger(char const* exePath) {
  auto const logFilePath =
      jwlrep::getExePath(exePath).replace_filename("logger.cfg");

  std::ifstream configFileStream(logFilePath.c_str());
  if (!configFileStream) {
    XLOGF(INFO,
          "Cannot open logger configuration file. Defaults will be used. File: "
          "{}",
          logFilePath.string());
    return jwlrep::GeneralError::Success;
  }
  auto const configContent =
      std::string{(std::istreambuf_iterator<char>(configFileStream)),
                  std::istreambuf_iterator<char>()};

  folly::LoggerDB::get().resetConfig(folly::parseLogConfig(configContent));
  return jwlrep::GeneralError::Success;
}

/**
 * Main entry point of the application.
 * @param argc Count of command line arguments.
 * @param argv Command line arguments.
 * @return Error code.
 */
int main(int argc, char** argv) {
  // Assume that in the first arg exe path is stored. On some platforms it's not
  // true;
  assert(argc > 0);
  assert(argv[0] != nullptr);
  folly::init(&argc, &argv, true); // We have to init folly internals (logger,
                                   // singleton etc)

  try {
    auto const loggerSetupError = setupLogger(argv[0]);
    if (loggerSetupError) {
      XLOGF(ERR, "Failed to setup logger: {}", loggerSetupError.message());
      return loggerSetupError.value();
    }

    XLOGF(INFO, "Starting app ver {}", jwlrep::getVersion());

    XLOG(DBG, "This is Debug");
    XLOG(WARN, "This is Warning");

    auto const appConfigOrError = jwlrep::processCmdArgs(argc, argv);
    if (!appConfigOrError) {
      if (appConfigOrError.error() == jwlrep::GeneralError::Interrupted) {
        return make_error_code(jwlrep::GeneralError::Success).value();
      } else if (appConfigOrError.error() ==
                 jwlrep::GeneralError::WrongStartupParams) {
        XLOG(DBG, "Wrong startup paramer(s)");
        return appConfigOrError.error().value();
      } else {
        XLOGF(ERR,
              "Unhandled command line error: {}",
              appConfigOrError.error().message());
        return appConfigOrError.error().value();
      }
    }
    auto const appConfig = appConfigOrError.value();

    jwlrep::EngineLauncher engineLauncher(std::move(appConfig));

    const auto errorCode = engineLauncher.run();
    // const auto errorCode = make_error_code(GeneralError::Success);
    XLOGF(INFO, "App has finished with exit code: '{}'", errorCode.message());
    return errorCode.value();
  } catch (std::exception const& error) {
    XLOGF(ERR, "Exception has occurred: {}", error.what());
  } catch (...) {
    XLOG(ERR, "Unknown exception has occurred");
  }
  auto const errorCode = make_error_code(jwlrep::GeneralError::InternalError);
  XLOGF(ERR, "Unexpected exit: {}", errorCode.message());
  return errorCode.value();
}