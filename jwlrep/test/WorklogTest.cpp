// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Worklog.h>

#include <catch2/catch.hpp>

TEST_CASE("Worklog: valid user timesheet", "[Worklog]") {
  auto const worklogJsonStr = R"(
    {
    "worklog": [{
            "key": "Key1",
            "summary": "Summary1",
            "entries": [{
                    "id": 1992012,
                    "comment": "Some comment 1",
                    "timeSpent": 3600,
                    "author": "user1",
                    "created": 1604507259177

                }
            ],
            "fields": []
        }, {
            "key": "Key2",
            "summary": "Summary2",
            "entries": [{
                    "timeSpent": 9000,
                    "author": "user1",
                    "created": 1604507617794
                }, {
                    "timeSpent": 10800,
                    "author": "user1",
                    "created": 1604507743944
                }
            ]
        }, {
            "key": "Key3",
            "summary": "Some comment3",
            "entries": [{
                    "timeSpent": 10800,
                    "author": "user1",
                    "created": 1604507580475
                }, {
                    "timeSpent": 7200,
                    "author": "user1",
                    "created": 1604507697037
                }, {
                    "comment": "",
                    "timeSpent": 10800,
                    "author": "user1",
                    "created": 1604507841101
                }
            ]
        }
    ],
    "startDate": 1604181600000,
    "endDate": 1604786400000
  }
  )";
  auto const userTimersheetOrError =
      jwlrep::createUserTimersheetFromJson(worklogJsonStr);
  REQUIRE(userTimersheetOrError.has_value());
  auto const& userTimersheet = userTimersheetOrError.value();
  REQUIRE(userTimersheet.worklog().size() == 3u);
  REQUIRE(userTimersheet.worklog()[1u].entries().size() == 2u);
  REQUIRE(userTimersheet.worklog()[1u].entries()[1u].author() == "user1");
  REQUIRE(userTimersheet.worklog()[1u].entries()[1u].created().month() == 11);
}

TEST_CASE("Worklog: Invalid user timesheet. Bad Json.", "[Worklog]") {
  auto const worklogJsonStr = R"(
    {
    "worklog": [{
            "key" - "Key1",
            "summary": "Summary1",
            "entries": [{
                    "id": 1992012,
                    "comment": "Some comment 1",
                    "timeSpent": 3600,
                    "author": "user1",
                    "created": 1604507259177

                }
            ],
            "fields": []
        }
  )";
  auto const userTimersheetOrError =
      jwlrep::createUserTimersheetFromJson(worklogJsonStr);
  REQUIRE(userTimersheetOrError.has_error());
  REQUIRE(userTimersheetOrError.error() == std::errc::invalid_argument);
}

TEST_CASE("Worklog: Invalid user timesheet. Missing key", "[Worklog]") {
  auto const worklogJsonStr = R"(
  {
  "worklog": [{
      "summary": "Summary1",
      "entries": [{
              "id": 1992012,
              "comment": "Some comment 1",
              "timeSpent": 3600,
              "author": "user1",
              "created": 1604507259177

          }
      ],
      "fields": []
  }
  )";
  auto const userTimersheetOrError =
      jwlrep::createUserTimersheetFromJson(worklogJsonStr);
  REQUIRE(userTimersheetOrError.has_error());
  REQUIRE(userTimersheetOrError.error() == std::errc::invalid_argument);
}

TEST_CASE("Worklog: Invalid user timesheet. Missing summary", "[Worklog]") {
  auto const worklogJsonStr = R"(
  {
  "worklog": [{
      "key": "Key1",
      "entries": [{
              "id": 1992012,
              "comment": "Some comment 1",
              "timeSpent": 3600,
              "author": "user1",
              "created": 1604507259177

          }
      ],
      "fields": []
  }
  )";
  auto const userTimersheetOrError =
      jwlrep::createUserTimersheetFromJson(worklogJsonStr);
  REQUIRE(userTimersheetOrError.has_error());
  REQUIRE(userTimersheetOrError.error() == std::errc::invalid_argument);
}
