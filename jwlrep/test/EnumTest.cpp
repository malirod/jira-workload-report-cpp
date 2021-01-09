// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <catch2/catch.hpp>
#include <magic_enum.hpp>
#include <sstream>

namespace {

enum class EnumDefaultInit {
  Foo,
  Bar,
};

enum class EnumCustomInit {
  Foo = 1,
  Bar,
};

}  // namespace

TEST_CASE("To intergal", "[Enum]") {
  REQUIRE(magic_enum::enum_integer(EnumDefaultInit::Foo) == 0);
  REQUIRE(magic_enum::enum_integer(EnumDefaultInit::Bar) == 1);
  REQUIRE(magic_enum::enum_integer(EnumCustomInit::Foo) == 1);
  REQUIRE(magic_enum::enum_integer(EnumCustomInit::Bar) == 2);
}

TEST_CASE("From intergal", "[Enum]") {
  auto const defaultFooOrError = magic_enum::enum_cast<EnumDefaultInit>(0);
  REQUIRE(defaultFooOrError.has_value());
  REQUIRE(EnumDefaultInit::Foo == defaultFooOrError.value());

  auto const defaultBarOrError = magic_enum::enum_cast<EnumDefaultInit>(1);
  REQUIRE(defaultBarOrError.has_value());
  REQUIRE(EnumDefaultInit::Bar == defaultBarOrError.value());

  auto const customFooOrError = magic_enum::enum_cast<EnumCustomInit>(1);
  REQUIRE(customFooOrError.has_value());
  REQUIRE(EnumCustomInit::Foo == customFooOrError.value());

  auto const customBarOrError = magic_enum::enum_cast<EnumCustomInit>(2);
  REQUIRE(customBarOrError.has_value());
  REQUIRE(EnumCustomInit::Bar == customBarOrError.value());
}

TEST_CASE("To string stream without init", "[Enum]") {
  using magic_enum::ostream_operators::operator<<;

  std::stringstream sstream;
  sstream << EnumDefaultInit::Foo << "," << EnumDefaultInit::Bar;

  REQUIRE("Foo,Bar" == sstream.str());
}

TEST_CASE("To string stream with init", "[Enum]") {
  using magic_enum::ostream_operators::operator<<;

  std::stringstream sstream;
  sstream << EnumCustomInit::Foo << "," << EnumCustomInit::Bar;

  REQUIRE("Foo,Bar" == sstream.str());
}

TEST_CASE("From valid string", "[Enum]") {
  auto const enumString = std::string{"Bar"};
  auto const barOrError = magic_enum::enum_cast<EnumCustomInit>(enumString);
  REQUIRE(barOrError.has_value());
  REQUIRE(barOrError.value() == EnumCustomInit::Bar);
}

TEST_CASE("From invalid string", "[Enum]") {
  auto const enumString = std::string{"Buzz"};
  auto const valueOrError = magic_enum::enum_cast<EnumCustomInit>(enumString);
  REQUIRE(!valueOrError.has_value());
}

TEST_CASE("To string custom init", "[Enum]") {
  REQUIRE("Foo" == magic_enum::enum_name(EnumDefaultInit::Foo));
  REQUIRE("Bar" == magic_enum::enum_name(EnumDefaultInit::Bar));
}

TEST_CASE("To string for invalid enum item", "[Enum]") {
  REQUIRE(!magic_enum::enum_cast<EnumCustomInit>(0).has_value());
  REQUIRE(!magic_enum::enum_cast<EnumCustomInit>(3).has_value());
}
