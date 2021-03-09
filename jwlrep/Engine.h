// SPDX-License-Identifier: MIT

// Copyright (C) 2021 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/AppConfig.h>
#include <jwlrep/Worklog.h>

#include <boost/asio/spawn.hpp>
#include <boost/beast/ssl.hpp>

#include "boost/asio/any_io_executor.hpp"

namespace jwlrep {

class Engine final {
 public:
  explicit Engine(boost::asio::any_io_executor ioExecutor,
                  AppConfig const& appConfig);

  auto operator=(Engine const&) -> Engine& = delete;
  Engine(Engine const&) = delete;

  using CompletionHandler = std::function<void()>;
  using OnStoppedEventHandler = std::function<void()>;

  void startAsync(CompletionHandler onDone);

 private:
  auto loadTimesheets(boost::asio::yield_context& yield)
      -> Expected<TimeSheets>;

  void generateTimesheetsXSLTReport(TimeSheets const& timeSheets);

  boost::asio::any_io_executor ioExecutor_;

  boost::asio::ssl::context sslContext_;

  AppConfig const& appConfig_;
};

}  // namespace jwlrep
