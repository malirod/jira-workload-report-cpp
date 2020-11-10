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
  Entry(std::chrono::seconds const& timeSpent,
        std::string const& author,
        boost::gregorian::date const& created);

  std::chrono::seconds const& timeSpent() const;

  std::string const& author() const;

  boost::gregorian::date const& created() const;

 private:
  std::chrono::seconds timeSpent_;

  std::string author_;

  boost::gregorian::date created_;
};

class Worklog {
 public:
  Worklog(std::string const& key,
          std::string const& summary,
          std::vector<Entry>&& entries);

  std::string const& key() const;

  std::string const& summary() const;

  std::vector<Entry> const& entries() const;

 private:
  std::string key_;

  std::string summary_;

  std::vector<Entry> entries_;
};

class UserTimersheet {
 public:
  explicit UserTimersheet(std::vector<Worklog>&& worklog);

  std::vector<Worklog> const& worklog() const;

 private:
  std::vector<Worklog> worklog_;
};

Expected<UserTimersheet> createUserTimersheetFromJson(
    std::string const& userTimersheetJsonStr);

} // namespace jwlrep