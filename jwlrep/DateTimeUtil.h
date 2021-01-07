// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

namespace jwlrep {

auto dateTimeFromMSecSinceEpoch(boost::posix_time::milliseconds msec)
    -> boost::posix_time::ptime;

}  // namespace jwlrep