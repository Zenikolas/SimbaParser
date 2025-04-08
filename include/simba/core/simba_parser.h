#pragma once

#include "simba/core/assembler.h"
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
  SimbaAssembler assembler_;
  ReassemblyState reassembly_state_ = ReassemblyState::None;

  void handle_stream_transition(bool is_incremental);
  bool buffer_fragment(uint16_t msg_flags, const uint8_t *payload,
                       size_t payload_len);
  void process_complete_message(bool is_incremental);
};

} // namespace simba
