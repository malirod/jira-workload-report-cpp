// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <nlohmann/json-schema.hpp>
#include <string>

namespace jwlrep {

class JsonValidatorErrorHandler
    : public nlohmann::json_schema::basic_error_handler {
  void error(nlohmann::json_pointer<nlohmann::basic_json<>> const& pointer,
             nlohmann::json const& instance,
             std::string const& message) override;
};

}  // namespace jwlrep