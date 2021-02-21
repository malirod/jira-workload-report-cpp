// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Engine.h>
#include <jwlrep/EngineLauncher.h>
#include <jwlrep/Logger.h>
#include <jwlrep/ScopeGuard.h>
#include <jwlrep/SignalHandler.h>

namespace jwlrep {

EngineLauncher::EngineLauncher(AppConfig appConfig)
    : appConfig_(std::move(appConfig)) {}

void EngineLauncher::onTerminationRequest() {
  LOG_INFO("Termination request received. Stopping.");
  engine_->stop();
}

void EngineLauncher::onEngineStarted() { LOG_DEBUG("Engine started"); }

void EngineLauncher::onEngineStopped() {
  LOG_DEBUG("Engine stopped");
  ioContext_->stop();
}

auto EngineLauncher::init() -> std::error_code {
  ioContext_ = std::make_shared<boost::asio::io_context>();

  signalHandler_ =
      std::make_unique<SignalHandler>([this]() { onTerminationRequest(); });
  signalHandler_->install(*ioContext_, {SIGINT, SIGTERM});

  engine_ = std::make_unique<Engine>(ioContext_, *this, appConfig_);

  LOG_DEBUG("Initiated");
  return GeneralError::Success;
}

void EngineLauncher::deInit() {
  assert(ioContext_->stopped());
  LOG_DEBUG("Deinitiated");
}

auto EngineLauncher::doRun() -> std::error_code {
  engine_->start();

  LOG_INFO("Waiting for termination request");

  boost::system::error_code errorCode;
  ioContext_->run(errorCode);
  if (errorCode) {
    LOG_DEBUG("Main execution loop has finished with error: {}",
              errorCode.message());
    return GeneralError::InternalError;
  }

  return GeneralError::Success;
}

auto EngineLauncher::run() -> std::error_code {
  const auto errorCode = init();
  auto const guard = ScopeGuard{[&]() { deInit(); }};
  return errorCode ? errorCode : doRun();
}

}  // namespace jwlrep