#pragma once

#include <string>
#include <vector>

#include "simba/core/parsed_message.h"
#include "simba/core/simba_messages.h"

namespace simba {

class JsonFormatter {
public:
  static std::string format(const SBEHeader &msg);
  static std::string format(const OrderUpdate &msg);
  static std::string format(const OrderExecution &msg);
  static std::string format(const OrderBookSnapshot &msg);
  static std::string format(const OrderBookEntry &msg);
  static std::string format(const ParsedMessage &msg);
};
} // namespace simba