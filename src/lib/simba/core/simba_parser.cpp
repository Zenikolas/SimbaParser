#include "simba/core/simba_parser.h"

#include "simba/core/decoder.h"
#include "simba/core/market_data.h"
#include "simba/core/read_struct.h"

#include "iostream"

namespace simba {

void SimbaParser::feed(const uint8_t *data, size_t len) {
  if (len < sizeof(MarketDataPacketHeader)) {
    std::cerr << "Packet too short for MarketDataPacketHeader\n";
    return;
  }

  const auto header = read_struct<MarketDataPacketHeader>(data);

  const bool is_last_fragment = (header.msg_flags & 0x01) != 0;
  const bool is_start_of_snapshot = (header.msg_flags & 0x02) != 0;
  const bool is_end_of_snapshot = (header.msg_flags & 0x04) != 0;
  const bool is_incremental = (header.msg_flags & 0x08) != 0;

  const uint8_t *payload = data + sizeof(MarketDataPacketHeader);
  size_t payload_len = len - sizeof(MarketDataPacketHeader);

  bool complete = false;

  if (is_incremental) {
    if (payload_len < sizeof(IncrementalPacketHeader)) {
      std::cerr << "Packet too short for IncrementalPacketHeader\n";
      return;
    }

    payload += sizeof(IncrementalPacketHeader);
    payload_len -= sizeof(IncrementalPacketHeader);

    assembler_.feed(payload, payload_len);

    complete = is_last_fragment;
  } else {
    if (is_start_of_snapshot) {
      assembler_.clear();
    }

    assembler_.feed(payload, payload_len);

    if (is_end_of_snapshot) {
      complete = true;
    }
  }

  if (!complete) {
    return;
  }

  auto simba_payload = assembler_.get_payload();
  const uint8_t *ptr = simba_payload.data();
  size_t remaining = simba_payload.size();

  if (!is_incremental) {
    // Snapshot: only one message per packet
    if (remaining < sizeof(SBEHeader)) {
      std::cerr << "Truncated snapshot SBE header\n";
      return;
    }

    const auto sbe = read_struct<SBEHeader>(ptr);
    remaining -= sizeof(SBEHeader);
    ptr += sizeof(SBEHeader);

    if (remaining < sbe.block_length) {
      std::cerr << "Truncated snapshot body, remaining= " << remaining
                << " sizeof(SBEHeader)=" << sizeof(SBEHeader)
                << " sbe.block_length=" << sbe.block_length << "\n";
      return;
    }

    auto result = decode_message(sbe.template_id, ptr, remaining);
    if (result)
      callback_(*result);
    return;
  }

  // Incremental: multiple messages
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
