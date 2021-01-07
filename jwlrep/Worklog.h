// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/Outcome.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>
#include <vector>

namespace jwlrep {

class Entry {
 public:
  Entry(std::chrono::seconds timeSpent, std::string author,
        boost::gregorian::date created);

  [[nodiscard]] auto timeSpent() const -> std::chrono::seconds const&;

  [[nodiscard]] auto author() const -> std::string const&;

  [[nodiscard]] auto created() const -> boost::gregorian::date const&;

 private:
  std::chrono::seconds timeSpent_;

  std::string author_;

  boost::gregorian::date created_;
};

class Worklog {
 public:
  Worklog(std::string key, std::string summary, std::vector<Entry>&& entries);

  [[nodiscard]] auto key() const -> std::string const&;

  [[nodiscard]] auto summary() const -> std::string const&;

  [[nodiscard]] auto entries() const -> std::vector<Entry> const&;

 private:
  std::string key_;

  std::string summary_;

  std::vector<Entry> entries_;
};

class UserTimeSheet {
 public:
  explicit UserTimeSheet(std::vector<Worklog>&& worklog);

  [[nodiscard]] auto worklog() const -> std::vector<Worklog> const&;

 private:
  std::vector<Worklog> worklog_;
};

using TimeSheets = std::vector<UserTimeSheet>;

auto createUserTimeSheetFromJson(std::string const& userTimeSheetJsonStr)
    -> Expected<UserTimeSheet>;

}  // namespace jwlrep