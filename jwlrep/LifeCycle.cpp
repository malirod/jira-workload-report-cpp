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
auto main(int argc, char** argv) -> int {
  auto errorCode = make_error_code(jwlrep::GeneralError::Success);
  try {
    jwlrep::setupLogger(argc, argv);

    LOG_INFO("Starting app ver {}", jwlrep::kProjectVersion);

    auto const appConfigOrError = jwlrep::processCmdArgs(argc, argv);
    if (!appConfigOrError) {
      if (appConfigOrError.error() == jwlrep::GeneralError::Interrupted) {
        return make_error_code(jwlrep::GeneralError::Success).value();
      }

      if (appConfigOrError.error() ==
          jwlrep::GeneralError::WrongStartupParams) {
        LOG_DEBUG("Wrong startup paramer(s)");
        return appConfigOrError.error().value();
      }

      LOG_ERROR("Unhandled command line error: {}",
                appConfigOrError.error().message());
      return appConfigOrError.error().value();
    }
    auto appConfig = appConfigOrError.value();
    jwlrep::EngineLauncher engineLauncher(std::move(appConfig));

    errorCode = engineLauncher.run();
  } catch (std::exception const& error) {
    LOG_ERROR("Exception has occurred: {}", error.what());
    errorCode = make_error_code(jwlrep::GeneralError::InternalError);
  } catch (...) {
    LOG_ERROR("Unknown exception has occurred");
    errorCode = make_error_code(jwlrep::GeneralError::InternalError);
  }
  LOG_INFO("App has finished with exit code: '{}'", errorCode.message());
  return errorCode.value();
}