// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Engine.h>
#include <jwlrep/EngineLauncher.h>
#include <jwlrep/ScopeGuard.h>
#include <jwlrep/SignalHandler.h>

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/fibers/FiberManager.h>
#include <folly/fibers/FiberManagerMap.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/logging/xlog.h>

#include <csignal>

namespace jwlrep {

EngineLauncher::EngineLauncher(AppConfig appConfig)
    : appConfig_(std::move(appConfig)) {
}

void EngineLauncher::onTerminationRequest() {
  XLOG(INFO, "Termination request received. Stopping.");
  stopped_ = true;
  mainEventBase_->terminateLoopSoon();
  engine_->stop();
}

void EngineLauncher::onEngineStarted() {
  XLOG(INFO, "Engine started");
  assert(mainEventBase_ != nullptr);
}

void EngineLauncher::onEngineStopped() {
  XLOG(INFO, "Engine stopped");
}

std::error_code EngineLauncher::init() {
  XLOGF(INFO, "Jira: {}", appConfig_.credentials.server);

  signalHandler_ =
      std::make_unique<SignalHandler>([this]() { onTerminationRequest(); });
  signalHandler_->install({SIGINT, SIGTERM});

  // Setup CPU executor
  // auto cpuThreadExecutor = std::make_shared<folly::CPUThreadPoolExecutor>(
  //     startupConfig_.threadsCount,
  //     std::make_shared<folly::NamedThreadFactory>("CPUThread"));
  // folly::setCPUExecutor(cpuThreadExecutor);

  mainEventBase_ = folly::EventBaseManager::get()->getEventBase();
  fiberManager_ = &folly::fibers::getFiberManager(*mainEventBase_);

  engine_ = std::make_unique<Engine>(*mainEventBase_, *fiberManager_, *this);

  auto const initiated = engine_->init();
  return initiated ? GeneralError::Success : GeneralError::StartupFailed;
}

void EngineLauncher::deInit() {
  mainEventBase_ = nullptr;
}

std::error_code EngineLauncher::doRun() {
  assert(mainEventBase_ != nullptr);

  engine_->start();

  XLOG(INFO, "Waiting for termination request");
  mainEventBase_->loopForever();
  // Manual event loop (alternative to mainEventBase_->loopForever())
  // while (!stopped_) {
  // mainEventBase_->loopOnce(EVLOOP_NONBLOCK);
  // engine_->processEvents();
  //}

  return GeneralError::Success;
}

std::error_code EngineLauncher::run() {
  const auto errorCode = init();
  jwlrep::ScopeGuard const guard{[&]() { deInit(); }};
  return errorCode ? errorCode : doRun();
}

} // namespace jwlrep