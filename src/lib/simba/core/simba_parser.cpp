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
  if (len < sizeof(MarketDataPacketHeader)) [[unlikely]] {
    std::cerr << "Packet too short for MarketDataPacketHeader\n";
    return;
  }

  const auto header = read_struct<MarketDataPacketHeader>(data);
  const uint16_t msg_flags = header.msg_flags;

  const uint8_t *payload = data + sizeof(MarketDataPacketHeader);
  size_t payload_len = len - sizeof(MarketDataPacketHeader);

  const bool is_incremental = (msg_flags & 0x08) != 0;

  if (is_incremental) {
    if (payload_len < sizeof(IncrementalPacketHeader)) [[unlikely]] {
      std::cerr << "Packet too short for IncrementalPacketHeader\n";
      return;
    }

    payload += sizeof(IncrementalPacketHeader);
    payload_len -= sizeof(IncrementalPacketHeader);
  }

  process_message(is_incremental, payload, payload_len);
}

void SimbaParser::process_message(bool is_incremental, const uint8_t *ptr,
                                  size_t remaining) {
  if (!is_incremental) {
    if (remaining < sizeof(SBEHeader)) [[unlikely]] {
      std::cerr << "Truncated snapshot SBE header\n";
      return;
    }

    const auto sbe = read_struct<SBEHeader>(ptr);
    ptr += sizeof(SBEHeader);
    remaining -= sizeof(SBEHeader);

    if (remaining < sbe.block_length) [[unlikely]] {
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

    if (sbe.block_length == 0 || remaining < sbe.block_length) [[unlikely]] {
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
