// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Engine.h>

#include <jwlrep/IEngineEventHandler.h>

#include <folly/fibers/FiberManager.h>
#include <folly/io/async/EventBase.h>
#include <folly/logging/xlog.h>

#include <cassert>
#include <utility>

namespace {

} // namespace

namespace jwlrep {

Engine::Engine(folly::EventBase& mainEventBase,
               folly::fibers::FiberManager& fiberManager,
               IEngineEventHandler& engineEventHandler)
    : mainEventBase_(mainEventBase),
      fiberManager_(fiberManager),
      engineEventHandler_(engineEventHandler) {
  XLOG(INFO, "Engine has been created.");
}

Engine::~Engine() {

  stopped_ = true;

  XLOG(INFO, "Engine has been destroyed.");
}

void Engine::start() {
  XLOG(INFO, "Starting engine");
  assert(initiated_);

  mainEventBase_.runInLoop([this]() { engineEventHandler_.onEngineStarted(); });

  stopped_ = false;

  XLOG(INFO, "Engine has been launched.");

  fiberManager_.addTask([] { XLOG(INFO, "Executing from Fiber"); });

  return;
}

void Engine::stop() {
  XLOG(INFO, "Stopping engine");
  assert(initiated_);

  if (stopped_) {
    XLOG(INFO, "Already stopped. Skip.");
    return;
  }

  stopped_ = true;
  mainEventBase_.runInLoop([this]() { engineEventHandler_.onEngineStopped(); });

  return;
}

bool Engine::init() {
  assert(!initiated_);

  initiated_ = true;
  return initiated_;
}

} // namespace jwlrep