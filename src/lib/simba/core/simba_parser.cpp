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
  const bool is_incremental = (header.msg_flags & 0x08) != 0;
  const bool is_last_fragment = (header.msg_flags & 0x01) != 0;

  const uint8_t *payload = data + sizeof(MarketDataPacketHeader);
  size_t payload_len = len - sizeof(MarketDataPacketHeader);

  if (is_incremental) {
    if (payload_len < sizeof(IncrementalPacketHeader)) {
      std::cerr << "Packet too short for IncrementalPacketHeader\n";
      return;
    }

    payload += sizeof(IncrementalPacketHeader);
    payload_len -= sizeof(IncrementalPacketHeader);
  }

  // Assemble fragments (if fragmented)
  bool complete = assembler_.feed(payload, payload_len, !is_last_fragment);
  if (!complete)
    return;

  auto simba_payload = assembler_.get_payload();
  const uint8_t *ptr = simba_payload.data();
  size_t remaining = simba_payload.size();

  // Loop over SBE messages inside the packet
  while (remaining >= sizeof(SBEHeader)) {
    const auto sbe = read_struct<SBEHeader>(ptr);
    size_t message_len = sizeof(SBEHeader) + sbe.block_length;

    if (remaining < message_len) {
      std::cerr << "Truncated SBE message\n";
      return;
    }

    auto result = decode_message(ptr, message_len);
    if (result) {
      callback_(*result);
    }

    ptr += message_len;
    remaining -= message_len;
  }
}

} // namespace simba
