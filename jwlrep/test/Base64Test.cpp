// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Base64.h>

#include <catch2/catch.hpp>

TEST_CASE("Empty", "[Base64]") {
  REQUIRE(jwlrep::base64Encode("") == "");
}

TEST_CASE("Http basic auth (demo:p@55w0rd)", "[Base64]") {
  auto const plainUserNameAndPwd = "demo:p@55w0rd";
  REQUIRE(jwlrep::base64Encode(plainUserNameAndPwd) == "ZGVtbzpwQDU1dzByZA==");
}

TEST_CASE("Http basic auth (none:none)", "[Base64]") {
  auto const plainUserNameAndPwd = "none:none";
  REQUIRE(jwlrep::base64Encode(plainUserNameAndPwd) == "bm9uZTpub25l");
}

TEST_CASE("Http basic auth (aladdin:opensesame)", "[Base64]") {
  auto const plainUserNameAndPwd = "aladdin:opensesame";
  REQUIRE(jwlrep::base64Encode(plainUserNameAndPwd) ==
          "YWxhZGRpbjpvcGVuc2VzYW1l");
}