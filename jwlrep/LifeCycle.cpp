// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/EngineLauncher.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/Logger.h>
#include <jwlrep/PathUtil.h>
#include <jwlrep/Version.h>

#include <cassert>
#include <fstream>

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

  auto errorCode = make_error_code(jwlrep::GeneralError::Success);
  try {
    jwlrep::setupLogger(argc, argv);

    SPDLOG_DEBUG("Starting app ver {}", jwlrep::getVersion());

    auto const appConfigOrError = jwlrep::processCmdArgs(argc, argv);
    if (!appConfigOrError) {
      if (appConfigOrError.error() == jwlrep::GeneralError::Interrupted) {
        return make_error_code(jwlrep::GeneralError::Success).value();
      } else if (appConfigOrError.error() ==
                 jwlrep::GeneralError::WrongStartupParams) {
        SPDLOG_DEBUG("Wrong startup paramer(s)");
        return appConfigOrError.error().value();
      } else {
        SPDLOG_ERROR("Unhandled command line error: {}",
                     appConfigOrError.error().message());
        return appConfigOrError.error().value();
      }
    }
    auto const appConfig = appConfigOrError.value();
    jwlrep::EngineLauncher engineLauncher(std::move(appConfig));

    errorCode = engineLauncher.run();
  } catch (std::exception const& error) {
    SPDLOG_ERROR("Exception has occurred: {}", error.what());
    errorCode = make_error_code(jwlrep::GeneralError::InternalError);
  } catch (...) {
    SPDLOG_ERROR("Unknown exception has occurred");
    errorCode = make_error_code(jwlrep::GeneralError::InternalError);
  }
  SPDLOG_INFO("App has finished with exit code: '{}'", errorCode.message());
  return errorCode.value();
}