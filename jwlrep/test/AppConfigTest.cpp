// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/GeneralError.h>

#include <catch2/catch.hpp>

TEST_CASE("Valid config", "[AppConfig]") {
  auto const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "weekNum": 41,
        "users": ["User1", "User2"]
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfig(config);
  REQUIRE(appConfigOrError.has_value());
  REQUIRE(appConfigOrError.value().credentials.server == "my.server.com");
}

TEST_CASE("Invalid config. Bad Json.", "[AppConfig]") {
  auto const config = R"(
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "weekNum": 41,
        "users": ["User1", "User2"]
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfig(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing server.", "[AppConfig]") {
  auto const config = R"(
    {
      "credentials": {
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "weekNum": 41,
        "users": ["User1", "User2"]
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfig(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing userName.", "[AppConfig]") {
  auto const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "password":"PASSWORD"
      },
      "options": {
        "weekNum": 41,
        "users": ["User1", "User2"]
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfig(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing password.", "[AppConfig]") {
  auto const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN"
      },
      "options": {
        "weekNum": 41,
        "users": ["User1", "User2"]
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfig(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing weekNum.", "[AppConfig]") {
  auto const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "users": ["User1", "User2"]
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfig(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing users.", "[AppConfig]") {
  auto const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "weekNum": 41
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfig(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. WeekNum must be int.", "[AppConfig]") {
  auto const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "weekNum": "some_string",
        "users": ["User1", "User2"]
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfig(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}