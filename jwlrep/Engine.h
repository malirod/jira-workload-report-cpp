// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/AppConfig.h>
#include <jwlrep/IEngineEventHandler.h>
#include <jwlrep/Worklog.h>

#include <boost/asio/io_context.hpp>
#include <boost/beast/ssl.hpp>

namespace jwlrep {

/**
 * Implementation of Engine. Runs all business logic.
 */
class Engine final {
 public:
  explicit Engine(std::shared_ptr<boost::asio::io_context> ioContext,
                  IEngineEventHandler& engineEventHandler,
                  AppConfig const& appConfig);

  auto operator=(Engine const&) -> Engine& = delete;
  Engine(Engine const&) = delete;

  /**
   * Start Engine. Non-blocking call. Actual startup will be performed
   * asynchronously.
   */
  void start();

  /**
   * Trigger stop sequence. Non-blocking.
   */
  void stop();

 private:
  auto loadTimesheets() -> Expected<TimeSheets>;

  void generateTimesheetsXSLTReport(TimeSheets const& timeSheets);

  std::shared_ptr<boost::asio::io_context> ioContext_;

  std::unique_ptr<boost::asio::ssl::context> sslContext_;

  IEngineEventHandler& engineEventHandler_;

  AppConfig const& appConfig_;
};

}  // namespace jwlrep
