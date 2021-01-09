// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/GeneralError.h>

#include <catch2/catch.hpp>
#include <magic_enum.hpp>

using jwlrep::GeneralError;

TEST_CASE("Enum to string", "[GeneralErrorEnum]") {
  REQUIRE("Success" == magic_enum::enum_name(GeneralError::Success));
  REQUIRE(make_error_code(GeneralError::Interrupted).message() ==
          magic_enum::enum_name(GeneralError::Interrupted));
}

TEST_CASE("To integral", "[GeneralErrorEnum]") {
  REQUIRE(0 == magic_enum::enum_integer(GeneralError::Success));
  REQUIRE(make_error_code(GeneralError::InternalError).value() ==
          magic_enum::enum_integer(GeneralError::InternalError));
}

TEST_CASE("Makers", "[GeneralErrorEnum]") {
  REQUIRE(make_error_code(GeneralError::Interrupted).value() != 0);
}
