// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/AppConfig.h>
#include <jwlrep/Engine.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/IEngineEventHandler.h>
#include <jwlrep/SignalHandler.h>

#include <boost/asio/io_context.hpp>
#include <memory>

namespace jwlrep {

/**
 * Encapsulates logic of initialization of Engine and startup.
 */
class EngineLauncher final : public IEngineEventHandler {
 public:
  /**
   * Create ready to use instance of EngineLauncher.
   * @param appConfig Configuration.
   */
  explicit EngineLauncher(AppConfig appConfig);

  EngineLauncher(EngineLauncher const&) = delete;
  EngineLauncher(EngineLauncher const&&) = delete;
  auto operator=(EngineLauncher const&) -> EngineLauncher& = delete;
  auto operator=(EngineLauncher const&&) -> EngineLauncher& = delete;

  /**
   * Setup environment, run Engine and wait until it will finish.
   * @return Execution result.
   */
  auto run() -> std::error_code;

 private:
  auto init() -> std::error_code;

  void deInit();

  auto doRun() -> std::error_code;

  void onTerminationRequest();

  void onEngineStarted() override;

  void onEngineStopped() override;

  AppConfig const appConfig_;

  /**
   * Signal handler which will shutdown Engine
   */
  std::unique_ptr<SignalHandler> signalHandler_;

  std::unique_ptr<Engine> engine_;

  std::shared_ptr<boost::asio::io_context> ioContext_;
};

}  // namespace jwlrep
