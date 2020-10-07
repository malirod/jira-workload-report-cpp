# SPDX-License-Identifier: MIT

# Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

cmake_minimum_required(VERSION 3.18)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake")
include(DefaultDebug)

project(jwlrep VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set global warnings configuration
#add_compile_options(-W4  -Wno-unused-command-line-argument -Wno-ignored-attributes)
#add_compile_options(-W4 -WX)
add_compile_options(-W4)

# Dependencies
find_package(fmt CONFIG REQUIRED)
find_package(folly CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

# Project lib configuration
set(LIB_NAME jwlreplib)
set(LIB_SRC_LIST
    "jwlrep/GeneralError.h"
    "jwlrep/GeneralError.cpp"
    "jwlrep/Engine.h"
    "jwlrep/Engine.cpp"
    "jwlrep/EngineLauncher.h"
    "jwlrep/EngineLauncher.cpp"
    "jwlrep/AppConfig.h"
    "jwlrep/AppConfig.cpp"
    "jwlrep/Version.h"
    "jwlrep/Version.cpp"
    "jwlrep/PathUtil.h"
    "jwlrep/PathUtil.cpp"
    "jwlrep/EnumUtil.h"
    "jwlrep/SignalHandler.h"
    "jwlrep/SignalHandler.cpp"
    "jwlrep/IEngineEventHandler.h"
    "jwlrep/ScopeGuard.h"
)

add_library(${LIB_NAME} ${LIB_SRC_LIST})
add_library(jwlrep::${LIB_NAME} ALIAS ${LIB_NAME})

# make possible to include headers using "<project/file>" notation
target_include_directories(${LIB_NAME}
  BEFORE
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_20)
target_link_libraries(${LIB_NAME}
  PUBLIC
  ZLIB::ZLIB
  fmt::fmt
  Folly::folly
  Folly::folly_deps
)

# App configuration
set(APP_NAME jwlrep)
add_executable(${APP_NAME} "jwlrep/LifeCycle.cpp")
target_compile_features(${APP_NAME} PRIVATE cxx_std_20)
target_link_libraries(${APP_NAME} PRIVATE jwlrep::${LIB_NAME})

# Copy default config to the output dir
if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/logger.cfg)
  message(STATUS "Copy logger config to bin output dir")
  configure_file(config/logger.cfg logger.cfg COPYONLY)
else()
  message(STATUS "Skip copy logger config to bin output dir. Already exists.")
endif()

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.cfg)
  message(STATUS "Copy app config to bin output dir")
  configure_file(config/${CMAKE_PROJECT_NAME}.cfg ${CMAKE_PROJECT_NAME}.cfg COPYONLY)
else()
  message(STATUS "Skip copy app config to bin output dir. Already exists.")
endif()