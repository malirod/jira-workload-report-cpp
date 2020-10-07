// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/SignalHandler.h>

#include <boost/asio/io_service.hpp>

namespace jwlrep {

SignalHandler::SignalHandler(SignalHandlerCallback handler)
    : handler_(std::move(handler)) {
}

void SignalHandler::install(boost::asio::io_service& ioService,
                            std::vector<int> const& signals) {
  signals_ = std::make_unique<boost::asio::signal_set>(ioService);
  for (const int& signal : signals) {
    signals_->add(signal);
  }
  signals_->async_wait(
      [&](auto const&, auto signal) { signalReceived(signal); });
}

void SignalHandler::signalReceived(int) noexcept {
  handler_();
}

} // namespace jwlrep
