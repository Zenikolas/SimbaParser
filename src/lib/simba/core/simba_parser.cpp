#include "simba/core/simba_parser.h"
#include "simba/core/decoder.h"
#include "simba/core/market_data.h"
#include "simba/core/read_struct.h"

#include <iostream>
#include <utility>

namespace simba {
namespace {
template <typename F> struct ScopeGuard {
  F f;
  ScopeGuard(F func) : f(std::move(func)) {}
  ~ScopeGuard() { f(); }
};
} // namespace

SimbaParser::SimbaParser(Callback cb) : callback_(std::move(cb)) {}

void SimbaParser::feed(const uint8_t *data, size_t len) {
  if (len < sizeof(MarketDataPacketHeader)) {
    std::cerr << "Packet too short for MarketDataPacketHeader\n";
    return;
  }

  const auto header = read_struct<MarketDataPacketHeader>(data);
  const uint16_t msg_flags = header.msg_flags;

  const uint8_t *payload = data + sizeof(MarketDataPacketHeader);
  size_t payload_len = len - sizeof(MarketDataPacketHeader);

  const bool is_incremental = (msg_flags & 0x08) != 0;

  handle_stream_transition(is_incremental);

  if (!buffer_fragment(msg_flags, payload, payload_len)) {
    return;
  }

  process_complete_message(is_incremental);
}

void SimbaParser::handle_stream_transition(bool is_incremental) {
  if (is_incremental && reassembly_state_ == ReassemblyState::Snapshot) {
    std::cerr << "Received incremental during snapshot reassembly — discarding "
                 "snapshot buffer\n";
    assembler_.clear();
    reassembly_state_ = ReassemblyState::None;
  }

  if (!is_incremental && reassembly_state_ == ReassemblyState::Incremental) {
    std::cerr << "Received snapshot during incremental reassembly — discarding "
                 "incremental buffer\n";
    assembler_.clear();
    reassembly_state_ = ReassemblyState::None;
  }
}

bool SimbaParser::buffer_fragment(uint16_t msg_flags, const uint8_t *payload,
                                  size_t payload_len) {
  const bool is_incremental = (msg_flags & 0x08) != 0;
  const bool is_last = (msg_flags & 0x01) != 0;
  const bool is_start = (msg_flags & 0x02) != 0;
  const bool is_end = (msg_flags & 0x04) != 0;

  if (is_incremental) {
    if (payload_len < sizeof(IncrementalPacketHeader)) {
      std::cerr << "Packet too short for IncrementalPacketHeader\n";
      assembler_.clear();
      return false;
    }

    payload += sizeof(IncrementalPacketHeader);
    payload_len -= sizeof(IncrementalPacketHeader);

    assembler_.feed(payload, payload_len);
    reassembly_state_ =
        is_last ? ReassemblyState::None : ReassemblyState::Incremental;
  } else {
    if (!is_start && reassembly_state_ != ReassemblyState::Snapshot) {
      std::cerr << "Orphan snapshot fragment (no StartOfSnapshot)\n";
      assembler_.clear();
      return false;
    }

    if (is_start) {
      assembler_.clear();
      reassembly_state_ = ReassemblyState::Snapshot;
    }

    assembler_.feed(payload, payload_len);

    if (is_end) {
      reassembly_state_ = ReassemblyState::None;
    }
  }

  return reassembly_state_ == ReassemblyState::None;
}

void SimbaParser::process_complete_message(bool is_incremental) {
  auto simba_payload = assembler_.get_payload();
  auto clear_on_exit = ScopeGuard([this] { assembler_.clear(); });

  const uint8_t *ptr = simba_payload.data();
  size_t remaining = simba_payload.size();

  if (!is_incremental) {
    if (remaining < sizeof(SBEHeader)) {
      std::cerr << "Truncated snapshot SBE header\n";
      return;
    }

    const auto sbe = read_struct<SBEHeader>(ptr);
    ptr += sizeof(SBEHeader);
    remaining -= sizeof(SBEHeader);

    if (remaining < sbe.block_length) {
      std::cerr << "Truncated snapshot body\n";
      return;
    }

    auto result = decode_message(sbe.template_id, ptr, remaining);
    if (result)
      callback_(*result);
    return;
  }

  while (remaining >= sizeof(SBEHeader)) {
    const auto sbe = read_struct<SBEHeader>(ptr);
    ptr += sizeof(SBEHeader);
    remaining -= sizeof(SBEHeader);

    if (sbe.block_length == 0 || remaining < sbe.block_length) {
      std::cerr << "Truncated incremental message\n";
      return;
    }

    auto result = decode_message(sbe.template_id, ptr, sbe.block_length);
    if (result)
      callback_(*result);

    ptr += sbe.block_length;
    remaining -= sbe.block_length;
  }
}

} // namespace simba
