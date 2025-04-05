#pragma once

#include "simba_messages.h"

namespace simba {
using ParsedMessage =
    std::variant<OrderUpdate, OrderExecution, OrderBookSnapshot>;
} // namespace simba