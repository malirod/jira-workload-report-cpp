// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/EnumUtil.h>
#include <jwlrep/ErrorCodeUtil.h>

#include <boost/asio/error.hpp>
#include <catch2/catch.hpp>

TEST_CASE("Convert general boost error code to appropriate std error code",
          "[ErrorCodeUtil]") {
  boost::system::error_code const boostErrorCode =
      make_error_code(boost::system::errc::no_such_device_or_address);

  std::error_code const stdErrorCode = jwlrep::toStd(boostErrorCode);
  REQUIRE(stdErrorCode.value() == boostErrorCode.value());
  REQUIRE(stdErrorCode.category() == boostErrorCode.category());
}

TEST_CASE("Convert system boost error code to std::error_code",
          "[ErrorCodeUtil]") {
  boost::system::error_code const boostErrorCode =
      make_error_code(boost::asio::error::network_down);

  std::error_code const stdErrorCode = jwlrep::toStd(boostErrorCode);
  REQUIRE(stdErrorCode.value() == boostErrorCode.value());
  REQUIRE(stdErrorCode.category() == boostErrorCode.category());
}

TEST_CASE("General boost error code is not equal to appropriate std error code",
          "[ErrorCodeUtil]") {
  REQUIRE(make_error_code(std::errc::no_such_device_or_address) !=
          jwlrep::toStd(
              make_error_code(boost::system::errc::no_such_device_or_address)));
}