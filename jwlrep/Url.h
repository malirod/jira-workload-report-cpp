// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <jwlrep/Outcome.h>

#include <optional>
#include <string>

namespace jwlrep {

class Url {
 public:
  using Port = std::optional<uint32_t>;

  static auto create(std::string const& urlString) -> Expected<Url>;

  [[nodiscard]] auto scheme() const -> std::string const&;

  [[nodiscard]] auto host() const -> std::string const&;

  [[nodiscard]] auto port() const -> Port;

 private:
  Url(std::string scheme, std::string host, Port port);

  std::string scheme_;

  std::string host_;

  Port port_;
};

}  // namespace jwlrep