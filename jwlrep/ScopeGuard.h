// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <functional>
#include <utility>

namespace jwlrep {

/**
 * Class uses RAII idiom for automatic cleanup.
 */
class ScopeGuard {
 public:
  using CleanupAction = std::function<void()>;
  /**
   * Create guard object.
   * @param cleanupAction action to be performed when going out of scope.
   */
  explicit ScopeGuard(CleanupAction cleanupAction)
      : cleanupAction_(std::move(cleanupAction)) {
  }

  /**
   * Destroy object and call cleanup action..
   */
  ~ScopeGuard() {
    cleanupAction_();
  }

 private:
  CleanupAction const cleanupAction_;
};

} // namespace jwlrep
