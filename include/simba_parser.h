#pragma once
#include "simba_messages.h"

#include <vector>
#include <cstdint>

namespace simba {

// Parse raw binary data into a list of OrderUpdate messages (stub for now)
std::vector<OrderUpdate> parse(const std::vector<uint8_t>& rawData);

}  // namespace simba
