// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/Worklog.h>

namespace jwlrep {

class Options;

void createReportExcel(TimeSheets const& timeSheets, Options const& options);

}  // namespace jwlrep