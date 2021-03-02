// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fmt/ostream.h>
#include <jwlrep/Engine.h>
#include <jwlrep/EngineLauncher.h>
#include <jwlrep/Logger.h>

#include <boost/fiber/all.hpp>
#include <boost/range/irange.hpp>
#include <cstdint>
#include <thread>

namespace jwlrep {

EngineLauncher::EngineLauncher(AppConfig appConfig)
    : appConfig_(std::move(appConfig)) {}

auto EngineLauncher::run() -> std::error_code {
  std::mutex mtx;
  boost::fibers::condition_variable_any cnd;
  auto const kThreadPoolSize = std::thread::hardware_concurrency();

  auto threadFn = [&mtx, &cnd, &kThreadPoolSize]() {
    LOG_DEBUG("Thread started {}", std::this_thread::get_id());
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::work_stealing>(
        kThreadPoolSize);
    std::unique_lock<std::mutex> lock(mtx);
    cnd.wait(lock);
  };

  auto completionFn = [&cnd]() {
    LOG_DEBUG("Engine stopped");
    cnd.notify_all();
  };

  std::vector<std::thread> threads;
  threads.reserve(kThreadPoolSize - 1);
  for (auto i : boost::irange<unsigned>(0, kThreadPoolSize - 1)) {
    threads.emplace_back(threadFn);
  }

  boost::fibers::use_scheduling_algorithm<boost::fibers::algo::work_stealing>(
      kThreadPoolSize);

  Engine engine{appConfig_};
  engine.start(std::move(completionFn));

  LOG_DEBUG("!!! Waiting main thread");
  {
    std::unique_lock<std::mutex> lock(mtx);
    cnd.wait(lock);
  }
  LOG_DEBUG("!!! Waited main thread");

  for (auto& thread : threads) {
    LOG_DEBUG("!!! Joining thread");
    thread.join();
  }
  LOG_DEBUG("!!! Joined all threads");

  return GeneralError::Success;
}

}  // namespace jwlrep