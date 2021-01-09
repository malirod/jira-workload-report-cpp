// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/DateTimeUtil.h>
#include <jwlrep/GeneralError.h>
#include <jwlrep/Logger.h>
#include <jwlrep/Worklog.h>

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

namespace nlohmann {
template <>
struct adl_serializer<jwlrep::Entry> {
  static auto from_json(json const& json) -> jwlrep::Entry {
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
  static auto from_json(json const& json) -> jwlrep::Worklog {
    return jwlrep::Worklog{json["key"].get<std::string>(),
                           json["summary"].get<std::string>(),
                           json["entries"].get<std::vector<jwlrep::Entry>>()};
    ;
  }
};

template <>
struct adl_serializer<jwlrep::UserTimeSheet> {
  static auto from_json(json const& json) -> jwlrep::UserTimeSheet {
    return jwlrep::UserTimeSheet{
        json["worklog"].get<std::vector<jwlrep::Worklog>>()};
    ;
  }
};

}  // namespace nlohmann

namespace {

auto isJsonValid(nlohmann::json const& json) -> bool {
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
        jsonSchema, nullptr,
        nlohmann::json_schema::default_string_format_check);
    // TODO(malirod): Extract error handler which will not use exceptions. Make
    // util class.
    validator.validate(json);
  } catch (std::exception const& e) {
    LOG_ERROR("Worklog validation has failed: {}", e.what());
    return false;
  }
  return true;
}

}  // namespace

namespace jwlrep {
using boost::gregorian::date;

auto createUserTimeSheetFromJson(std::string const& userTimeSheetJsonStr)
    -> Expected<UserTimeSheet> {
  auto const userTimeSheetJson =
      nlohmann::json::parse(userTimeSheetJsonStr, nullptr, false, true);

  if (userTimeSheetJson.is_discarded()) {
    LOG_ERROR("Failed to parse worklog: json is not valid:\n{}",
              userTimeSheetJsonStr);
    return make_error_code(std::errc::invalid_argument);
  }

  if (!isJsonValid(userTimeSheetJson)) {
    LOG_ERROR("Worklog json is not valid:\n{}", userTimeSheetJsonStr);
    return make_error_code(std::errc::invalid_argument);
  }
  return userTimeSheetJson.get<UserTimeSheet>();
}

Worklog::Worklog(std::string key, std::string summary,
                 std::vector<Entry>&& entries)
    : key_(std::move(key)),
      summary_(std::move(summary)),
      entries_(std::move(entries)) {}

auto Worklog::key() const -> std::string const& { return key_; }

auto Worklog::summary() const -> std::string const& { return summary_; }

auto Worklog::entries() const -> std::vector<Entry> const& { return entries_; }

Entry::Entry(std::chrono::seconds timeSpent, std::string author,
             boost::gregorian::date created)
    : timeSpent_(timeSpent), author_(std::move(author)), created_(created) {}

auto Entry::timeSpent() const -> std::chrono::seconds const& {
  return timeSpent_;
}

auto Entry::author() const -> std::string const& { return author_; }

auto Entry::created() const -> date const& { return created_; }

UserTimeSheet::UserTimeSheet(std::vector<Worklog>&& worklog)
    : worklog_(std::move(worklog)) {}

auto UserTimeSheet::worklog() const -> std::vector<Worklog> const& {
  return worklog_;
}

}  // namespace jwlrep
