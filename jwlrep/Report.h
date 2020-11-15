// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <vector>

namespace jwlrep {

class UserTimeSheet;
class Options;

void createReportExcel(
    std::vector<std::reference_wrapper<UserTimeSheet>> const& timeSheets,
    Options const& options);

} // namespace jwlrep