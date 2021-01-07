// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/Logger.h>
#include <spdlog/async.h>
#include <spdlog/cfg/argv.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace jwlrep {

void setupLogger(int argc, char** argv) {
  auto const kThreadPoolQueueSize = 8192;
  auto const kPoolThreadsCount = 1;
  spdlog::init_thread_pool(kThreadPoolQueueSize, kPoolThreadsCount);
  auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  stdoutSink->set_level(spdlog::level::debug);
  // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  stdoutSink->set_pattern("%^[%H:%M:%S.%e][%t][%P][%L][%@]:%$ %v");
  std::vector<spdlog::sink_ptr> sinks{stdoutSink};
  auto logger = std::make_shared<spdlog::async_logger>(
      "main", sinks.begin(), sinks.end(), spdlog::thread_pool(),
      spdlog::async_overflow_policy::block);
  spdlog::register_logger(logger);
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::debug);
  spdlog::cfg::load_argv_levels(argc,
                                argv);  // ./example
                                        // SPDLOG_LEVEL=info,mylogger=trace
}

}  // namespace jwlrep