// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Logger.h>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int main(int argc, char* argv[]) {
  jwlrep::setupLogger(argc, argv);

  int result = Catch::Session().run(argc, argv);

  return result;
}