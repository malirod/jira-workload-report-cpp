// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Worklog.h>

#include <jwlrep/DateTimeUtil.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/Logger.h>

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

#include <boost/optional.hpp>

namespace nlohmann {
template <>
struct adl_serializer<jwlrep::Entry> {
  static jwlrep::Entry from_json(json const& json) {
    return jwlrep::Entry{
        std::chrono::seconds{json["timeSpent"].get<std::uint32_t>()},
        json["author"].get<std::string>(),
        jwlrep::dateTimeFromMSecSinceEpoch(
            boost::posix_time::milliseconds(
                json["created"].get<std::uint64_t>()))
            .date()};
    ;
  }
};

template <>
struct adl_serializer<jwlrep::Worklog> {
  static jwlrep::Worklog from_json(json const& json) {
    return jwlrep::Worklog{json["key"].get<std::string>(),
                           json["summary"].get<std::string>(),
                           json["entries"].get<std::vector<jwlrep::Entry>>()};
    ;
  }
};

template <>
struct adl_serializer<jwlrep::UserTimersheet> {
  static jwlrep::UserTimersheet from_json(json const& json) {
    return jwlrep::UserTimersheet{
        json["worklog"].get<std::vector<jwlrep::Worklog>>()};
    ;
  }
};

} // namespace nlohmann

namespace {

bool isJsonValid(nlohmann::json const& json) {
  auto const jsonSchema = R"(
  {
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "required": [
      "worklog"
    ],
    "properties": {
      "worklog": {
        "type": "array",
        "items": {
          "type": "object",
          "required": [
            "key",
            "summary",
            "entries"
          ],
          "properties": {
            "key": {
              "type": "string"
            },
            "summary": {
              "type": "string"
            },
            "entries": {
              "type": "array",
              "items": {
                "type": "object",
                "required": [
                  "timeSpent",
                  "author",
                  "created"
                ],
                "properties": {
                  "timeSpent": {
                    "type": "number"
                  },
                  "author": {
                    "type": "string"
                  },
                  "created": {
                    "type": "number"
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  )"_json;

  try {
    static nlohmann::json_schema::json_validator const validator(
        jsonSchema,
        nullptr,
        nlohmann::json_schema::default_string_format_check);
    // TODO: Extract error handler which will not use exceptions. Make util
    // class.
    validator.validate(json);
  } catch (std::exception const& e) {
    SPDLOG_ERROR("Worklog validation has failed: {}", e.what());
    return false;
  }
  return true;
}

} // namespace

namespace jwlrep {

Expected<UserTimersheet> createUserTimersheetFromJson(
    std::string const& userTimersheetJsonStr) {
  auto const userTimersheetJson =
      nlohmann::json::parse(userTimersheetJsonStr, nullptr, false, true);

  if (userTimersheetJson.is_discarded()) {
    SPDLOG_ERROR("Failed to parse worklog: json is not valid");
    return make_error_code(std::errc::invalid_argument);
  }

  if (!isJsonValid(userTimersheetJson)) {
    return make_error_code(std::errc::invalid_argument);
  }
  return userTimersheetJson.get<UserTimersheet>();
}

Worklog::Worklog(std::string const& key,
                 std::string const& summary,
                 std::vector<Entry>&& entries)
    : key_(key), summary_(summary), entries_(std::move(entries)) {
}

std::string const& Worklog::key() const {
  return key_;
}

std::string const& Worklog::summary() const {
  return summary_;
}

std::vector<Entry> const& Worklog::entries() const {
  return entries_;
}

Entry::Entry(std::chrono::seconds const& timeSpent,
             std::string const& author,
             boost::gregorian::date const& created)
    : timeSpent_(timeSpent), author_(author), created_(created) {
}

std::chrono::seconds const& Entry::timeSpent() const {
  return timeSpent_;
}

std::string const& Entry::author() const {
  return author_;
}

boost::gregorian::date const& Entry::created() const {
  return created_;
}

UserTimersheet::UserTimersheet(std::vector<Worklog>&& worklog)
    : worklog_(std::move(worklog)) {
}

std::vector<Worklog> const& UserTimersheet::worklog() const {
  return worklog_;
}

} // namespace jwlrep
