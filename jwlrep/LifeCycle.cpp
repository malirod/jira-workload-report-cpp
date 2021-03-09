// SPDX-License-Identifier: MIT

// Copyright (C) 2021 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/Engine.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/Logger.h>
#include <jwlrep/PathUtil.h>
#include <jwlrep/ScopeGuard.h>
#include <jwlrep/Version.h>

#include <boost/asio/signal_set.hpp>
#include <boost/asio/thread_pool.hpp>
#include <cassert>
#include <fstream>

namespace jwlrep {

auto run(AppConfig const& appConfig) -> std::error_code {
  // Use defaut threads count: std::thread::hardware_concurrency() * 2
  boost::asio::thread_pool threadPool;

  auto handleStopRequest = [&threadPool]() {
    LOG_DEBUG("Stopping thread pool");
    threadPool.stop();
  };

  auto handleTerminationRequest = [&handleStopRequest]() {
    LOG_INFO("Termination request received. Stopping.");
    handleStopRequest();
  };

  boost::asio::signal_set signals{threadPool, SIGINT, SIGTERM};
  signals.async_wait(
      [&handleTerminationRequest](auto const& /*unused*/, auto /*unused*/) {
        handleTerminationRequest();
      });

  Engine engine{threadPool.get_executor(), appConfig};
  engine.startAsync([&handleStopRequest]() { handleStopRequest(); });

  LOG_INFO("Waiting for termination request");
  threadPool.join();

  return GeneralError::Success;
}

}  // namespace jwlrep

auto main(int argc, char** argv) -> int {
  auto errorCode = make_error_code(jwlrep::GeneralError::Success);
  try {
    jwlrep::setupLogger(argc, argv);

    LOG_INFO("Starting app ver {}", jwlrep::kProjectVersion);

    auto const appConfigOrError = jwlrep::processCmdArgs(argc, argv);
    if (!appConfigOrError) {
      if (appConfigOrError.error() == jwlrep::GeneralError::Interrupted) {
        return make_error_code(jwlrep::GeneralError::Success).value();
      }

      if (appConfigOrError.error() ==
          jwlrep::GeneralError::WrongStartupParams) {
        LOG_DEBUG("Wrong startup paramer(s)");
        return appConfigOrError.error().value();
      }

      LOG_ERROR("Unhandled command line error: {}",
                appConfigOrError.error().message());
      return appConfigOrError.error().value();
    }
    errorCode = run(appConfigOrError.value());
  } catch (std::exception const& error) {
    LOG_ERROR("Exception has occurred: {}", error.what());
    errorCode = make_error_code(jwlrep::GeneralError::InternalError);
  } catch (...) {
    LOG_ERROR("Unknown exception has occurred");
    errorCode = make_error_code(jwlrep::GeneralError::InternalError);
  }
  LOG_INFO("App has finished with exit code: '{}'", errorCode.message());
  return errorCode.value();
}