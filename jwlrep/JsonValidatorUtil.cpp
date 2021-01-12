// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fmt/ostream.h>
#include <jwlrep/JsonValidatorUtil.h>
#include <jwlrep/Logger.h>

namespace jwlrep {

void JsonValidatorErrorHandler::error(
    nlohmann::json_pointer<nlohmann::basic_json<>> const& pointer,
    nlohmann::json const& instance, std::string const& message) {
  nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
  LOG_ERROR("Json validation has failed: '{}-{}: {}'", pointer, instance,
            message);
}

}  // namespace jwlrep