// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <utility>

namespace jwlrep {

/**
 * Class uses RAII idiom for automatic cleanup.
 */
template <typename Fn>
class ScopeGuard {
 public:
  /**
   * Create guard object.
   * @param cleanupAction action to be performed when going out of scope.
   */
  explicit ScopeGuard(Fn&& cleanupAction)
      : cleanupAction_(std::forward<Fn>(cleanupAction)) {}

  /**
   * Destroy object and call cleanup action..
   */
  ~ScopeGuard() { cleanupAction_(); }

 private:
  Fn cleanupAction_;
};

}  // namespace jwlrep
