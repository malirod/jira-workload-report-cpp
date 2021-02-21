// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <csignal>
#include <functional>
#include <vector>

namespace jwlrep {

/**
 * Installs signal handler which will stop App when the user presses
 * Ctrl-C.
 */
class SignalHandler {
 public:
  using SignalHandlerCallback = std::function<void()>;
  explicit SignalHandler(SignalHandlerCallback handler);

  ~SignalHandler() = default;

  void install(boost::asio::io_service& ioService,
               std::vector<int> const& signals);

 private:
  void signalReceived(int signum) noexcept;

  SignalHandlerCallback const handler_;

  std::unique_ptr<boost::asio::signal_set> signals_;
};

}  // namespace jwlrep
