// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/GeneralError.h>
#include <jwlrep/Logger.h>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

auto main(int argc, char* argv[]) -> int {
  auto errorCode = make_error_code(jwlrep::GeneralError::Success);
  try {
    jwlrep::setupLogger(argc, argv);

    int result = Catch::Session().run(argc, argv);

    if (result != 0) {
      LOG_ERROR("Test suite returned error: '{}'", result);
      errorCode = make_error_code(jwlrep::GeneralError::InternalError);
    }
  } catch (std::exception const& error) {
    LOG_ERROR("Exception has occurred: {}", error.what());
    errorCode = make_error_code(jwlrep::GeneralError::InternalError);
  } catch (...) {
    LOG_ERROR("Unknown exception has occurred");
    errorCode = make_error_code(jwlrep::GeneralError::InternalError);
  }
  return errorCode.value();
}