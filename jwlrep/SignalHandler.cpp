// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/SignalHandler.h>

#include <folly/io/async/EventBaseManager.h>

using folly::EventBaseManager;

namespace jwlrep {

SignalHandler::SignalHandler(SignalHandlerCallback handler)
    : folly::AsyncSignalHandler(EventBaseManager::get()->getEventBase()),
      handler_(std::move(handler)) {
}

void SignalHandler::install(std::vector<int> const& signals) {
  for (const int& signal : signals) {
    registerSignalHandler(signal);
  }
}

void SignalHandler::signalReceived(int) noexcept {
  handler_();
}

} // namespace jwlrep
