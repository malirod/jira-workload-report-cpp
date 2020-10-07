// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/AppConfig.h>
#include <jwlrep/Engine.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/IEngineEventHandler.h>
#include <jwlrep/SignalHandler.h>

#include <memory>

namespace folly {

class EventBase;
namespace fibers {

class FiberManager;

} // namespace fibers

} // namespace folly

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
  EngineLauncher& operator=(EngineLauncher const&) = delete;
  EngineLauncher& operator=(EngineLauncher const&&) = delete;

  /**
   * Setup environment, run Engine and wait until it will finish.
   * @return Execution result.
   */
  std::error_code run();

 private:
  std::error_code init();

  void deInit();

  std::error_code doRun();

  void onTerminationRequest();

  void onEngineStarted() override;

  void onEngineStopped() override;

  AppConfig const appConfig_;

  /**
   * Signal handler which will shutdown Engine
   */
  std::unique_ptr<SignalHandler> signalHandler_;

  std::unique_ptr<Engine> engine_;

  folly::EventBase* mainEventBase_ = nullptr;

  folly::fibers::FiberManager* fiberManager_ = nullptr;

  bool stopped_ = false;
};

} // namespace jwlrep