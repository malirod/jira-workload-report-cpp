// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Engine.h>
#include <jwlrep/EngineLauncher.h>
#include <jwlrep/Logger.h>
#include <jwlrep/ScopeGuard.h>
#include <jwlrep/SignalHandler.h>

#include <csignal>

namespace jwlrep {

EngineLauncher::EngineLauncher(AppConfig appConfig)
    : appConfig_(std::move(appConfig)) {
}

void EngineLauncher::onTerminationRequest() {
  SPDLOG_INFO("Termination request received. Stopping.");
  engine_->stop();
}

void EngineLauncher::onEngineStarted() {
  SPDLOG_DEBUG("Engine started");
}

void EngineLauncher::onEngineStopped() {
  SPDLOG_DEBUG("Engine stopped");
}

std::error_code EngineLauncher::init() {
  SPDLOG_INFO("Jira: {}", appConfig_.credentials().server());

  ioContext_ = std::make_shared<boost::asio::io_service>();

  signalHandler_ =
      std::make_unique<SignalHandler>([this]() { onTerminationRequest(); });
  signalHandler_->install(*ioContext_, {SIGINT, SIGTERM});

  engine_ = std::make_unique<Engine>(ioContext_, *this, appConfig_);

  auto const initiated = engine_->init();
  return initiated ? GeneralError::Success : GeneralError::StartupFailed;
}

void EngineLauncher::deInit() {
}

std::error_code EngineLauncher::doRun() {
  engine_->start();

  SPDLOG_INFO("Waiting for termination request");

  boost::system::error_code errorCode;
  ioContext_->run(errorCode);
  if (errorCode) {
    SPDLOG_DEBUG("Main execution loop is done with error: {}",
                 errorCode.message());
    return GeneralError::InternalError;
  }

  return GeneralError::Success;
}

std::error_code EngineLauncher::run() {
  const auto errorCode = init();
  jwlrep::ScopeGuard const guard{[&]() { deInit(); }};
  return errorCode ? errorCode : doRun();
}

} // namespace jwlrep