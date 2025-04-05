#pragma once
#include "market_data.h"
#include "parsed_message.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace simba {

class SimbaParser {
public:
  using MessageCallback = std::function<void(const ParsedMessage &)>;

  explicit SimbaParser(MessageCallback cb) : callback_(std::move(cb)) {}

  void feed(const uint8_t *data, size_t len);

private:
  MessageCallback callback_;
};

} // namespace simba
