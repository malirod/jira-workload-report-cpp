// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

namespace jwlrep {

struct IEngineEventHandler {
  virtual ~IEngineEventHandler() = default;

  virtual void onEngineStarted() = 0;

  virtual void onEngineStopped() = 0;
};

}  // namespace jwlrep