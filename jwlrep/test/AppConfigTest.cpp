// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/GeneralError.h>

#include <catch2/catch.hpp>

TEST_CASE("Valid config", "[AppConfig]") {
  const auto *const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "dateStart": "2020-11-21",
        "dateEnd": "2020-12-23",
        "users": ["User1", "User2"],
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_value());
  REQUIRE(appConfigOrError.value().credentials().server() == "my.server.com");
  REQUIRE(appConfigOrError.value().options().dateStart().year() == 2020);
  REQUIRE(appConfigOrError.value().options().dateStart().month() == 11);
  REQUIRE(appConfigOrError.value().options().dateStart().day() == 21);
  REQUIRE(appConfigOrError.value().options().dateEnd().year() == 2020);
  REQUIRE(appConfigOrError.value().options().dateEnd().month() == 12);
  REQUIRE(appConfigOrError.value().options().dateEnd().day() == 23);
  REQUIRE(appConfigOrError.value().options().defaultAssociation() == "SOP");
  REQUIRE(appConfigOrError.value().options().associations().size() == 2);
}

TEST_CASE("Associations keys are normalized during load", "[AppConfig]") {
  const auto *const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "dateStart": "2020-11-21",
        "dateEnd": "2020-12-23",
        "users": ["User1", "User2"],
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";

  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_value());
  REQUIRE(appConfigOrError.value().options().associations().size() == 2);
  REQUIRE(appConfigOrError.value()
              .options()
              .associations()
              .find("[common]")
              ->second == "Common");
  REQUIRE(appConfigOrError.value()
              .options()
              .associations()
              .find("[arch]")
              ->second == "Non-SOP");
}

TEST_CASE("Invalid config. Bad Json.", "[AppConfig]") {
  const auto *const config = R"(
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "dateStart": "2020-11-21",
        "dateEnd": "2020-11-23",
        "users": ["User1", "User2"],
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing server.", "[AppConfig]") {
  const auto *const config = R"(
    {
      "credentials": {
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "dateStart": "2020-11-21",
        "dateEnd": "2020-11-23",
        "users": ["User1", "User2"],
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing userName.", "[AppConfig]") {
  const auto *const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "password":"PASSWORD"
      },
      "options": {
        "dateStart": "2020-11-21",
        "dateEnd": "2020-11-23",
        "users": ["User1", "User2"],
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing password.", "[AppConfig]") {
  const auto *const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN"
      },
      "options": {
        "dateStart": "2020-11-21",
        "dateEnd": "2020-11-23",
        "users": ["User1", "User2"],
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing dateStart.", "[AppConfig]") {
  const auto *const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "dateEnd": "2020-11-23",
        "users": ["User1", "User2"],
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing dateEnd.", "[AppConfig]") {
  const auto *const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "dateStart: "2020-11-21",
        "users": ["User1", "User2"],
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. Missing users.", "[AppConfig]") {
  const auto *const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "dateStart": "2020-11-21",
        "dateEnd": "2020-11-23",
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. DateStart wrong format", "[AppConfig]") {
  const auto *const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "dateStart": "2020-Nov-21",
        "dateEnd": "2020-11-23",
        "users": ["User1", "User2"],
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}

TEST_CASE("Invalid config. DateEnd wrong format", "[AppConfig]") {
  const auto *const config = R"(
    {
      "credentials": {
        "server":"my.server.com",
        "userName":"LOGIN",
        "password":"PASSWORD"
      },
      "options": {
        "dateStart": "2020-11-21",
        "dateEnd": "2020-Nov-23",
        "users": ["User1", "User2"],
        "defaultAssociation": "SOP",
        "associations": {"[Common]": "Common", "[Arch]": "Non-SOP"}
      }
    }
  )";
  auto const appConfigOrError = jwlrep::createAppConfigFromJson(config);
  REQUIRE(appConfigOrError.has_error());
  REQUIRE(appConfigOrError.error() == jwlrep::GeneralError::InvalidAppConfig);
}