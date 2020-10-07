// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <folly/io/async/AsyncSignalHandler.h>
#include <functional>
#include <vector>

namespace jwlrep {

/**
 * Installs signal handler which will stop App when the user presses
 * Ctrl-C.
 */
class SignalHandler : private folly::AsyncSignalHandler {
 public:
  using SignalHandlerCallback = std::function<void()>;
  explicit SignalHandler(SignalHandlerCallback const handler);

  ~SignalHandler() override = default;

  void install(std::vector<int> const& signals);

 private:
  void signalReceived(int signum) noexcept override;

  SignalHandlerCallback const handler_;
};

} // namespace jwlrep
