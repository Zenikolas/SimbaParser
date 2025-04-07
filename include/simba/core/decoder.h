#pragma once

#include "simba/core/parsed_message.h"

#include <cstdint>
#include <optional>

namespace simba {
std::optional<ParsedMessage> decode_message(uint16_t template_id,
                                            const uint8_t *data, size_t len);
} // namespace simba