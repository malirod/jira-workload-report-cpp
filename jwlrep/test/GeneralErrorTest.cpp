// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/EnumUtil.h>
#include <jwlrep/GeneralError.h>

#include <catch2/catch.hpp>

using jwlrep::EnumToString;
using jwlrep::GeneralError;
using jwlrep::ToIntegral;

TEST_CASE("Enum to string", "[GeneralErrorEnum]") {
  REQUIRE("Success" == EnumToString(GeneralError::Success));
  REQUIRE(make_error_code(GeneralError::Interrupted).message() ==
          EnumToString(GeneralError::Interrupted));
}

TEST_CASE("To integral", "[GeneralErrorEnum]") {
  REQUIRE(0 == ToIntegral(GeneralError::Success));
  REQUIRE(make_error_code(GeneralError::InternalError).value() ==
          ToIntegral(GeneralError::InternalError));
}

TEST_CASE("Makers", "[GeneralErrorEnum]") {
  REQUIRE(make_error_code(GeneralError::Interrupted).value() != 0);
}
