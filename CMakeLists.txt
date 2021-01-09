# SPDX-License-Identifier: MIT

# Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

cmake_minimum_required(VERSION 3.18)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake")

project(
  jwlrep
  LANGUAGES CXX
  VERSION "1.0.0")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

if(WIN32)
  # Set global warnings configuration for all sub-projects
  if(MSVC)
    add_compile_options(-W4)
  else()
    add_compile_options(-Wall -Wextra)
  endif()

  # Prevent Boost compleins about Win Version
  macro(get_WIN32_WINNT version)
    if(CMAKE_SYSTEM_VERSION)
      set(ver ${CMAKE_SYSTEM_VERSION})
      string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
      string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
      # Check for Windows 10, b/c we'll need to convert to hex 'A'.
      if("${verMajor}" MATCHES "10")
        set(verMajor "A")
        string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
      endif()
      # Remove all remaining '.' characters.
      string(REPLACE "." "" ver ${ver})
      # Prepend each digit with a zero.
      string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
      set(${version} "0x${ver}")
    endif()
  endmacro()

  get_win32_winnt(ver)
  add_definitions(-D_WIN32_WINNT=${ver})
endif()

# Dependencies
find_package(Boost REQUIRED program_options fiber)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(nlohmann_json_schema_validator REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(xlnt REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

configure_file(config/Version.h.in jwlrep/Version.h)

# Project lib configuration
set(LIB_NAME jwlreplib)
set(LIB_SRC_LIST
    ${CMAKE_CURRENT_BINARY_DIR}/jwlrep/Version.h
    "jwlrep/GeneralError.h"
    "jwlrep/GeneralError.cpp"
    "jwlrep/Engine.h"
    "jwlrep/Engine.cpp"
    "jwlrep/EngineLauncher.h"
    "jwlrep/EngineLauncher.cpp"
    "jwlrep/AppConfig.h"
    "jwlrep/AppConfig.cpp"
    "jwlrep/PathUtil.h"
    "jwlrep/PathUtil.cpp"
    "jwlrep/SignalHandler.h"
    "jwlrep/SignalHandler.cpp"
    "jwlrep/IEngineEventHandler.h"
    "jwlrep/ScopeGuard.h"
    "jwlrep/Outcome.h"
    "jwlrep/Logger.h"
    "jwlrep/Logger.cpp"
    "jwlrep/Base64.h"
    "jwlrep/Base64.cpp"
    "jwlrep/RootCertificates.h"
    "jwlrep/NetUtil.h"
    "jwlrep/NetUtil.cpp"
    "jwlrep/ErrorCodeUtil.h"
    "jwlrep/ErrorCodeUtil.cpp"
    "jwlrep/Worklog.h"
    "jwlrep/Worklog.cpp"
    "jwlrep/DateTimeUtil.h"
    "jwlrep/DateTimeUtil.cpp"
    "jwlrep/ExcelReport.h"
    "jwlrep/ExcelReport.cpp"
    "jwlrep/FiberUtil.h")

add_library(${LIB_NAME} ${LIB_SRC_LIST})
add_library(jwlrep::${LIB_NAME} ALIAS ${LIB_NAME})

# Set static linking (the value is ignored on non-MSVC compilers)
set_property(
  TARGET ${LIB_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
                              "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# make possible to include headers using "<project/file>" notation
target_include_directories(
  ${LIB_NAME} BEFORE PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)

# boost fiber asio placed in the project root available
target_include_directories(${LIB_NAME} PRIVATE "boost")

# To be able to include Version.h
target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
  ${LIB_NAME}
  PRIVATE fmt::fmt
          Boost::program_options
          Boost::fiber
          nlohmann_json::nlohmann_json
          nlohmann_json_schema_validator
          OpenSSL::SSL
          OpenSSL::Crypto
          xlnt::xlnt
          magic_enum::magic_enum)

if(MSVC)
  target_link_libraries(${LIB_NAME} INTERFACE Crypt32.lib)
endif()

target_link_libraries(${LIB_NAME} PUBLIC spdlog::spdlog)

# App configuration
set(APP_NAME jwlrep)
add_executable(${APP_NAME} "jwlrep/LifeCycle.cpp")
target_compile_features(${APP_NAME} PRIVATE cxx_std_17)
target_link_libraries(${APP_NAME} PRIVATE jwlrep::${LIB_NAME})

# Set static linking (the value is ignored on non-MSVC compilers)
set_property(
  TARGET ${APP_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
                              "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.cfg)
  message(STATUS "Copy app config to bin output dir")
  configure_file(config/${CMAKE_PROJECT_NAME}.cfg ${CMAKE_PROJECT_NAME}.cfg
                 COPYONLY)
else()
  message(STATUS "Skip copy app config to bin output dir. Already exists.")
endif()

if(BUILD_TESTING)
  find_package(Catch2 REQUIRED)

  set(TEST_LIB_NAME "${LIB_NAME}_test")

  set(TEST_SRC_LIST
      "jwlrep/test/AppConfigTest.cpp"
      "jwlrep/test/EnumTest.cpp"
      "jwlrep/test/GeneralErrorTest.cpp"
      "jwlrep/test/Base64Test.cpp"
      "jwlrep/test/ErrorCodeUtilTest.cpp"
      "jwlrep/test/WorklogTest.cpp"
      "jwlrep/test/ExcelReportTest.cpp")

  add_library(${TEST_LIB_NAME} OBJECT ${TEST_SRC_LIST})
  add_library(jwlrep::${TEST_LIB_NAME} ALIAS ${TEST_LIB_NAME})

  target_include_directories(${TEST_LIB_NAME} PRIVATE test)

  target_compile_features(${TEST_LIB_NAME} PRIVATE cxx_std_17)
  target_link_libraries(${TEST_LIB_NAME} PUBLIC jwlrep::${LIB_NAME}
                                                Catch2::Catch2)

  # Set static linking (the value is ignored on non-MSVC compilers)
  set_property(
    TARGET ${TEST_LIB_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
                                     "MultiThreaded$<$<CONFIG:Debug>:Debug>")

  set(TEST_RUNNER_NAME testrunner)

  add_executable(${TEST_RUNNER_NAME} "jwlrep/test/TestRunner.cpp")

  target_compile_features(${TEST_RUNNER_NAME} PRIVATE cxx_std_17)
  target_link_libraries(${TEST_RUNNER_NAME} PRIVATE ${TEST_LIB_NAME})

  # Set static linking (the value is ignored on non-MSVC compilers)
  set_property(
    TARGET ${TEST_RUNNER_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
                                        "MultiThreaded$<$<CONFIG:Debug>:Debug>")

  add_test(NAME all COMMAND ${TEST_RUNNER_NAME})

  include(Catch)
  catch_discover_tests(${TEST_RUNNER_NAME})
endif()
