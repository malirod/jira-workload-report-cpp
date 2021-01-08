// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/ExcelReport.h>

#include <catch2/catch.hpp>

namespace {
auto createOptions(
    std::string const& defaultAssociation,
    boost::container::flat_map<std::string, std::string>&& associations)
    -> jwlrep::Options {
  return jwlrep::Options{boost::gregorian::day_clock::universal_day(),
                         boost::gregorian::day_clock::universal_day(),
                         {},
                         defaultAssociation,
                         std::move(associations)};
}
}  // namespace

TEST_CASE("Default assosiation", "[ExcelReport]") {
  auto const options =
      createOptions("SOP", boost::container::flat_map<std::string, std::string>{
                               {"[Arch]", "Non-SOP"}});
  REQUIRE(jwlrep::calculateLabel("My Task", options) == "SOP");
}

TEST_CASE("Non-default assosiation. Normalization", "[ExcelReport]") {
  auto const options =
      createOptions("SOP", boost::container::flat_map<std::string, std::string>{
                               {"[arch]", "Non-SOP"}});
  REQUIRE(jwlrep::calculateLabel("[Arch]My Task", options) == "Non-SOP");
}

TEST_CASE("No key normalization. Fallback to default assosiation.",
          "[ExcelReport]") {
  auto const options =
      createOptions("SOP", boost::container::flat_map<std::string, std::string>{
                               {"[Arch]", "Non-SOP"}});
  REQUIRE(jwlrep::calculateLabel("[arch]My Task", options) == "SOP");
}