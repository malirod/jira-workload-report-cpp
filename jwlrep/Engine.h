// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <atomic>

namespace folly {

class EventBase;

namespace fibers {

class FiberManager;

} // namespace fibers

} // namespace folly

namespace jwlrep {

class RepeatableTimeout;

class AsyncServer;

struct IEngineEventHandler;

/**
 * Implementation of Engine. Holds all and runs all business logic.
 */
class Engine final {
 public:
  /**
   * Creates instance of Engine.
   * @param address Engine address.
   */
  explicit Engine(folly::EventBase& mainEventBase,
                  folly::fibers::FiberManager& fiberManager,
                  IEngineEventHandler& engineEventHandler);

  Engine& operator=(Engine const&) = delete;
  Engine(Engine const&) = delete;

  /**
   * Destroy Engine instance. Mark Engine as stopped. Don't perform actual
   * shutdown.
   */
  ~Engine();

  /**
   * Start Engine. Non-blocking call. Actual startup will be performed
   * asynchronously.
   */
  void start();

  /**
   * Trigger stop sequence. Non-blocking.
   */
  void stop();

  /**
   * Init Engine. Blocking call.
   * @return True if initiated and ready to go. False otherwise.
   */
  bool init();

 private:
  bool initiated_ = false;

  std::atomic_bool stopped_ = false;

  folly::EventBase& mainEventBase_;

  folly::fibers::FiberManager& fiberManager_;

  IEngineEventHandler& engineEventHandler_;
};

} // namespace jwlrep
