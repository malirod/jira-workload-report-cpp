// SPDX-License-Identifier: MIT

// Copyright (C) 2021 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/Base64.h>
#include <jwlrep/Engine.h>
#include <jwlrep/ExcelReport.h>
#include <jwlrep/Logger.h>
#include <jwlrep/NetUtil.h>
#include <jwlrep/RootCertificates.h>
#include <jwlrep/ScopeGuard.h>
#include <jwlrep/Worklog.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <cassert>
#include <magic_enum.hpp>
#include <utility>

namespace jwlrep {

Engine::Engine(boost::asio::any_io_executor ioExecutor,
               AppConfig const& appConfig)
    : ioExecutor_(std::move(ioExecutor)),
      sslContext_(boost::asio::ssl::context::tlsv12_client),
      appConfig_(appConfig) {
  LOG_DEBUG("Engine has been created.");

  loadRootCertificates(sslContext_);

  sslContext_.set_verify_mode(boost::asio::ssl::verify_peer);
}

void Engine::startAsync(CompletionHandler onDone) {
  LOG_DEBUG("Starting engine");

  auto doStartAsync =
      [this, onDone = std::move(onDone)](boost::asio::yield_context yield) {
        LOG_DEBUG("Executing main task");

        auto const guard = ScopeGuard{[&onDone]() {
          LOG_DEBUG("Main task completed.");
          onDone();
        }};

        try {
          auto const timeSheetsOrError = loadTimesheets(yield);

          if (!timeSheetsOrError) {
            LOG_ERROR("Failed to load timesheets: {}",
                      timeSheetsOrError.error().message());
            return;
          }

          generateTimesheetsXSLTReport(timeSheetsOrError.value());

        } catch (std::exception const& e) {
          LOG_ERROR("Got exception in main fiber: {}", e.what());
        }
      };

  LOG_DEBUG("Posting main task");
  // boost::asio::post(ioExecutor_, std::move(doStartAsync));
  boost::asio::spawn(ioExecutor_, std::move(doStartAsync));

  LOG_DEBUG("Engine has been launched.");
}

auto Engine::loadTimesheets(boost::asio::yield_context& yield)
    -> Expected<TimeSheets> {
  namespace http = boost::beast::http;

  auto makeHttpRequest = [&options = appConfig_.options(),
                          &credentials =
                              appConfig_.credentials()](auto const& userName) {
    auto const requestStr = fmt::format(
        "/rest/timesheet-gadget/1.0/"
        "raw-timesheet.json?targetUser={}&startDate={}&endDate={}",
        userName, boost::gregorian::to_iso_extended_string(options.dateStart()),
        boost::gregorian::to_iso_extended_string(options.dateEnd()));
    auto const kHTTPVersion = 10;
    http::request<http::empty_body> request{http::verb::get, requestStr,
                                            kHTTPVersion};

    request.set(
        http::field::authorization,
        "Basic " + base64Encode(fmt::format("{}:{}", credentials.userName(),
                                            credentials.password())));
    return request;
  };

  LOG_INFO(
      "Query timesheets for all users for time period: {} - {}",
      boost::gregorian::to_iso_extended_string(
          appConfig_.options().dateStart()),
      boost::gregorian::to_iso_extended_string(appConfig_.options().dateEnd()));

  auto dnsLookupResultsOrError =
      dnsLookup(ioExecutor_, appConfig_.credentials().serverUrl().host(),
                appConfig_.credentials().serverUrl().scheme(), yield);
  if (!dnsLookupResultsOrError) {
    LOG_ERROR("Failed to make dns lookup for url {}://{}. Error: {}",
              appConfig_.credentials().serverUrl().scheme(),
              appConfig_.credentials().serverUrl().host(),
              dnsLookupResultsOrError.error().message());
    return dnsLookupResultsOrError.error();
  }

  TimeSheets timeSheets;
  timeSheets.reserve(appConfig_.options().users().size());

  for (auto const& user : appConfig_.options().users()) {
    LOG_INFO("Requesting data for the user {}", user);
    boost::asio::spawn([&makeHttpRequest, &dnsLookupResultsOrError, &user,
                        &timeSheets, this](boost::asio::yield_context yield) {
      // auto const guard = ScopeGuard{[&]() {
      //  LOG_DEBUG("Request fiber has finished");
      //  barrier->wait();
      //}};
      auto request = makeHttpRequest(user);

      auto const responseOrError =
          httpGet(ioExecutor_, sslContext_, dnsLookupResultsOrError.value(),
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

      // timeSheets.emplace_back(std::move(userTimeSheetOrError.value()));

      LOG_INFO("Got data for the user {}", user);
    });
  }

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