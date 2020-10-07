// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/IEngineEventHandler.h>

#include <boost/asio/io_service.hpp>

namespace jwlrep {

/**
 * Implementation of Engine. Runs all business logic.
 */
class Engine final {
 public:
  explicit Engine(boost::asio::io_service& io_service,
                  IEngineEventHandler& engineEventHandler);

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
  bool initiated_ = false;

  boost::asio::io_service& ioService_;

  IEngineEventHandler& engineEventHandler_;
};

} // namespace jwlrep
