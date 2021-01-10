// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/GeneralError.h>
#include <jwlrep/Url.h>

#include <catch2/catch.hpp>

TEST_CASE("Invalid URL", "[URL]") {
  auto const urlOrError = jwlrep::Url::create("Not valid URL.");
  REQUIRE(urlOrError.has_error());
}

TEST_CASE("Valid URL with scheme, host and port", "[URL]") {
  auto const urlOrError = jwlrep::Url::create("https://my.site.com:8080");
  REQUIRE(urlOrError.has_value());
  auto const& url = urlOrError.value();
  REQUIRE(url.host() == "my.site.com");
  REQUIRE(url.scheme() == "https");
  REQUIRE(url.port().has_value());
  REQUIRE(url.port().value() == 8080);
}

TEST_CASE("Valid URL with empty port", "[URL]") {
  auto const urlOrError = jwlrep::Url::create("https://my.site.com");
  REQUIRE(urlOrError.has_value());
  auto const& url = urlOrError.value();
  REQUIRE(!url.port().has_value());
}

TEST_CASE("Invalid port fails Url creation", "[URL]") {
  auto const urlOrError = jwlrep::Url::create("https://my.site.com:80KK");
  REQUIRE(urlOrError.has_error());
}

TEST_CASE("Scheme is mandatory", "[URL]") {
  auto const urlOrError = jwlrep::Url::create("my.site.com");
  REQUIRE(urlOrError.has_error());
}

TEST_CASE("Host is mandatory", "[URL]") {
  auto const urlOrError = jwlrep::Url::create("https://");
  REQUIRE(urlOrError.has_error());
}