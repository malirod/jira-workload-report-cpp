// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/DateTimeUtil.h>

namespace jwlrep {

auto dateTimeFromMSecSinceEpoch(boost::posix_time::milliseconds msec)
    -> boost::posix_time::ptime {
  static const boost::posix_time::ptime epoch(
      boost::gregorian::date(1970, 1, 1));
  return epoch + msec;
}

}  // namespace jwlrep