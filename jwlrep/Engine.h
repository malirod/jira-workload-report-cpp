// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/AppConfig.h>
#include <jwlrep/Worklog.h>

namespace jwlrep {

class Engine final {
 public:
  explicit Engine(AppConfig const& appConfig);

  auto operator=(Engine const&) -> Engine& = delete;
  Engine(Engine const&) = delete;

  void start(std::function<void()> completionHandler);

 private:
  auto loadTimesheets() -> Expected<TimeSheets>;

  void generateTimesheetsXSLTReport(TimeSheets const& timeSheets);

  AppConfig const& appConfig_;
};

}  // namespace jwlrep
