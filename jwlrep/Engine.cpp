// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Engine.h>

#include <jwlrep/IEngineEventHandler.h>
#include <jwlrep/Logger.h>

#include <cassert>
#include <utility>

namespace jwlrep {

Engine::Engine(boost::asio::io_service& ioService,
               IEngineEventHandler& engineEventHandler)
    : ioService_(ioService), engineEventHandler_(engineEventHandler) {
  SPDLOG_DEBUG("Engine has been created.");
}

Engine::~Engine() {
  ioService_.stop();
  SPDLOG_DEBUG("Engine has been destroyed.");
}

void Engine::start() {
  SPDLOG_DEBUG("Starting engine");
  assert(initiated_);

  ioService_.post([this]() { engineEventHandler_.onEngineStarted(); });

  SPDLOG_DEBUG("Engine has been launched.");

  return;
}

void Engine::stop() {
  SPDLOG_DEBUG("Stopping engine");
  assert(initiated_);

  if (ioService_.stopped()) {
    SPDLOG_DEBUG("Already stopped. Skip.");
    return;
  }

  ioService_.post([this]() { engineEventHandler_.onEngineStopped(); });
  ioService_.stop();

  return;
}

bool Engine::init() {
  assert(!initiated_);

  initiated_ = true;
  return initiated_;
}

} // namespace jwlrep