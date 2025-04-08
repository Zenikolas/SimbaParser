#pragma once

#include "simba/core/parsed_message.h"

#include <cstdint>
#include <functional>

namespace simba {

class SimbaParser {
public:
  using Callback = std::function<void(const ParsedMessage &)>;

  explicit SimbaParser(Callback cb);

  void feed(const uint8_t *data, size_t len);

private:
  enum class ReassemblyState { None, Snapshot, Incremental };

  Callback callback_;

  void process_message(bool is_incremental, const uint8_t *ptr,
                       size_t remaining);
};

} // namespace simba
