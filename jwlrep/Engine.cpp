// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Engine.h>

#include <jwlrep/AppConfig.h>
#include <jwlrep/IEngineEventHandler.h>
#include <jwlrep/Logger.h>
#include <jwlrep/NetUtil.h>

#include <boost/asio/post.hpp>

#include <cassert>
#include <utility>

namespace jwlrep {

Engine::Engine(boost::asio::io_context& ioContext,
               IEngineEventHandler& engineEventHandler,
               AppConfig const& appConfig)
    : ioContext_(ioContext),
      engineEventHandler_(engineEventHandler),
      appConfig_(appConfig) {
  SPDLOG_DEBUG("Engine has been created.");
}

Engine::~Engine() {
  ioContext_.stop();
  SPDLOG_DEBUG("Engine has been destroyed.");
}

void Engine::start() {
  SPDLOG_DEBUG("Starting engine");
  assert(initiated_);

  boost::asio::post(ioContext_,
                    [this]() { engineEventHandler_.onEngineStarted(); });
  boost::asio::post(
      ioContext_, [this]() { auto const timeTimesheets = queryTimesheets(); });

  SPDLOG_DEBUG("Engine has been launched.");

  return;
}

void Engine::stop() {
  SPDLOG_DEBUG("Stopping engine");
  assert(initiated_);

  if (ioContext_.stopped()) {
    SPDLOG_DEBUG("Already stopped. Skip.");
    return;
  }

  boost::asio::post(ioContext_,
                    [this]() { engineEventHandler_.onEngineStopped(); });

  ioContext_.stop();

  return;
}

bool Engine::init() {
  assert(!initiated_);

  initiated_ = true;
  return initiated_;
}

std::vector<std::string> Engine::queryTimesheets() {
  SPDLOG_DEBUG("Query timesheets for all users");
  // auto const endpoints = resolveAsync(appConfig_.credentials.server);
  return std::vector<std::string>{};
}

} // namespace jwlrep