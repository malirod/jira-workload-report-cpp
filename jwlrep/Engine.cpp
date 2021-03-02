// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fmt/ostream.h>
#include <jwlrep/AppConfig.h>
#include <jwlrep/Engine.h>
#include <jwlrep/ExcelReport.h>
#include <jwlrep/Logger.h>
#include <jwlrep/NetUtil.h>
#include <jwlrep/ScopeGuard.h>
#include <jwlrep/Worklog.h>

#include <boost/fiber/all.hpp>
#include <cassert>
#include <magic_enum.hpp>
#include <utility>

#include "boost/fiber/operations.hpp"

namespace jwlrep {

Engine::Engine(AppConfig const& appConfig) : appConfig_(appConfig) {
  LOG_DEBUG("Engine has been created.");
}

void Engine::start(std::function<void()> completionHandler) {
  LOG_DEBUG("Starting engine");

  boost::fibers::fiber([this, completionHandler]() {
    LOG_DEBUG("Launched main fiber");
    LOG_DEBUG("Launched main fiber. Thread id: {}. Fiber id: {}",
              std::this_thread::get_id(), boost::this_fiber::get_id());

    try {
      auto const guard = ScopeGuard{[&completionHandler]() {
        LOG_DEBUG("Finished main fiber");
        completionHandler();
      }};

      auto const timeSheetsOrError = loadTimesheets();

      if (!timeSheetsOrError) {
        LOG_ERROR("Failed to load timesheets: {}",
                  timeSheetsOrError.error().message());
        return;
      }

      generateTimesheetsXSLTReport(timeSheetsOrError.value());

    } catch (std::exception const& e) {
      LOG_ERROR("Got exception in main fiber: {}", e.what());
    }
  }).detach();

  LOG_DEBUG("Engine has been launched.");
}

auto Engine::loadTimesheets() -> Expected<TimeSheets> {
  auto makeHttpRequest = [&options = appConfig_.options(),
                          &serverUrl = appConfig_.credentials().serverUrl()](
                             auto const& userName) {
    auto request = fmt::format(
        "{}/rest/timesheet-gadget/1.0/"
        "raw-timesheet.json?targetUser={}&startDate={}&endDate={}",
        serverUrl, userName,
        boost::gregorian::to_iso_extended_string(options.dateStart()),
        boost::gregorian::to_iso_extended_string(options.dateEnd()));

    return request;
  };

  LOG_INFO(
      "Query timesheets for all users for time period: {} - {}",
      boost::gregorian::to_iso_extended_string(
          appConfig_.options().dateStart()),
      boost::gregorian::to_iso_extended_string(appConfig_.options().dateEnd()));

  TimeSheets timeSheets;
  timeSheets.reserve(appConfig_.options().users().size());

  // It has to be shared ptr otherwise there will be the crash. Due to
  // cooperative mode outer function will exit (and barrier will be destroyed)
  // before last sub-fiber will exit
  auto barrier = std::make_shared<boost::fibers::barrier>(
      appConfig_.options().users().size() + 1);
  for (auto const& user : appConfig_.options().users()) {
    LOG_INFO("Requesting data for the user {}", user);
    boost::fibers::fiber([barrier, &user, &makeHttpRequest,
                          &credentials = appConfig_.credentials()]() {
      auto const guard = ScopeGuard{[&]() {
        LOG_DEBUG("Request fiber has finished");
        barrier->wait();
      }};
      LOG_DEBUG("Fiber for user {}. Thread id: {}. Fiber id: {}", user,
                std::this_thread::get_id(), boost::this_fiber::get_id());
      auto const responseOrError =
          httpGet(makeHttpRequest(user), credentials.userName(),
                  credentials.password(), std::chrono::seconds(10));
      if (!responseOrError) {
        LOG_ERROR("Failed to get data for user {}. Error: {}", user,
                  responseOrError.error().message());
        return;
      }
      LOG_DEBUG("Response: {}", responseOrError.value());
      auto userTimeSheetOrError =
          createUserTimeSheetFromJson(responseOrError.value());
      if (!userTimeSheetOrError) {
        LOG_ERROR("Failed to parse timesheet for user {}. Error: {}", user,
                  responseOrError.error().message());
        return;
      }
      // !!!!! push result to queue
    }).detach();
    /*
    boost::fibers::fiber([barrier, &makeHttpRequest, &dnsLookupResultsOrError,
                          &user, &yield, &timeSheets, this]() {
      auto const guard = ScopeGuard{[&]() {
        LOG_DEBUG("Request fiber has finished");
        barrier->wait();
      }};
      auto request = makeHttpRequest(user);

      auto const responseOrError =
          httpGet(*ioContext_, *sslContext_, dnsLookupResultsOrError.value(),
                  request, std::chrono::seconds(10), yield);
      if (!responseOrError) {
        LOG_ERROR("Failed to get data for user {}. Error: {}", user,
                  responseOrError.error().message());
        return;
      }
      if (responseOrError.value().result() != http::status::ok) {
        LOG_ERROR("Request has failed with result {}",
                  magic_enum::enum_integer(responseOrError.value().result()));
        return;
      }
      auto userTimeSheetOrError =
          createUserTimeSheetFromJson(responseOrError.value().body());
      if (!userTimeSheetOrError) {
        LOG_ERROR("Failed to parse timesheet for user {}. Error: {}", user,
                  responseOrError.error().message());
        return;
      }

      timeSheets.emplace_back(std::move(userTimeSheetOrError.value()));

    LOG_INFO("Got data for the user {}", user);
  }).detach();
  */
  }

  barrier->wait();
  LOG_INFO("All request have been finished.");
  return timeSheets;
}

void Engine::generateTimesheetsXSLTReport(TimeSheets const& timeSheets) {
  LOG_INFO("Generating report");
  if (timeSheets.empty()) {
    LOG_INFO("Timesheets are empty. Skip report generation.");
    return;
  }
  createReportExcel(timeSheets, appConfig_.options());
  LOG_INFO("Report has been saved");
}

}  // namespace jwlrep