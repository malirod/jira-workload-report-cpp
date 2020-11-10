// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/IEngineEventHandler.h>

#include <jwlrep/AppConfig.h>

#include <boost/asio/io_context.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/fiber/buffered_channel.hpp>

namespace jwlrep {

/**
 * Implementation of Engine. Runs all business logic.
 */
class Engine final {
 public:
  explicit Engine(std::shared_ptr<boost::asio::io_context> ioContext,
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
  void queryTimesheets();

  bool initiated_ = false;

  std::shared_ptr<boost::asio::io_context> ioContext_;

  std::unique_ptr<boost::asio::ssl::context> sslContext_;

  IEngineEventHandler& engineEventHandler_;

  AppConfig const& appConfig_;

  std::unique_ptr<boost::fibers::buffered_channel<int>> channel_;
};

} // namespace jwlrep
