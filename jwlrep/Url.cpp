// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/GeneralError.h>
#include <jwlrep/Logger.h>
#include <jwlrep/ScopeGuard.h>
#include <jwlrep/Url.h>
#include <uriparser/Uri.h>

namespace jwlrep {

Url::Url(std::string scheme, std::string host, Port port)
    : scheme_(std::move(scheme)), host_(std::move(host)), port_(port) {}

auto Url::scheme() const -> std::string const& { return scheme_; }

auto Url::host() const -> std::string const& { return host_; }

auto Url::port() const -> Port { return port_; }

auto Url::create(std::string const& urlString) -> Expected<Url> {
  auto fromRange = [](auto const& range) {
    return std::string{range.first, range.afterLast};
  };

  auto parsePort = [&fromRange](auto const& range) -> Port {
    auto const portStr = fromRange(range);
    if (portStr.empty()) {
      return std::nullopt;
    }
    try {
      return std::stoi(portStr);
    } catch (std::exception const& e) {
      LOG_ERROR("Failed to convert port {} to int: {}", portStr, e.what());
      return std::nullopt;
    }
  };

  const char* errorPos = nullptr;
  UriUriA uri;

  if (auto errorCode = uriParseSingleUriA(&uri, urlString.c_str(), &errorPos);
      errorCode != URI_SUCCESS) {
    LOG_ERROR("Failed to parse Url. Error code: {}.", errorCode);
    return GeneralError::WrongArg;
  }

  ScopeGuard const guard{[&uri]() { uriFreeUriMembersA(&uri); }};

  auto scheme = fromRange(uri.scheme);
  auto host = fromRange(uri.hostText);

  if (scheme.empty()) {
    LOG_ERROR("Scheme must not be empty in Url: {}.", urlString);
    return GeneralError::WrongArg;
  }

  if (host.empty()) {
    LOG_ERROR("Host must not be empty in Url: {}.", urlString);
    return GeneralError::WrongArg;
  }

  return Url{std::move(scheme), std::move(host), parsePort(uri.portText)};
}

}  // namespace jwlrep