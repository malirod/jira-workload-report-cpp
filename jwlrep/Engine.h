// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/IEngineEventHandler.h>

#include <boost/asio/io_context.hpp>

namespace jwlrep {

struct AppConfig;

/**
 * Implementation of Engine. Runs all business logic.
 */
class Engine final {
 public:
  explicit Engine(boost::asio::io_context& ioContext,
                  IEngineEventHandler& engineEventHandler,
                  AppConfig const& appConfig);

  Engine& operator=(Engine const&) = delete;
  Engine(Engine const&) = delete;

  /**
   * Destroy Engine instance. Mark Engine as stopped. Don't perform actual
   * shutdown.
   */
  ~Engine();

  /**
   * Start Engine. Non-blocking call. Actual startup will be performed
   * asynchronously.
   */
  void start();

  /**
   * Trigger stop sequence. Non-blocking.
   */
  void stop();

  /**
   * Init Engine. Blocking call.
   * @return True if initiated and ready to go. False otherwise.
   */
  bool init();

 private:
  std::vector<std::string> queryTimesheets();

  bool initiated_ = false;

  boost::asio::io_context& ioContext_;

  IEngineEventHandler& engineEventHandler_;

  AppConfig const& appConfig_;
};

} // namespace jwlrep
