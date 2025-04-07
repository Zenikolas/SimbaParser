#include "simba/core/decoder.h"

#include "simba/core/market_data.h"
#include "simba/core/parsed_message.h"
#include "simba/core/read_struct.h"

#include <iostream>

namespace simba {

std::optional<ParsedMessage>
decode_message(uint16_t template_id, const uint8_t *payload, size_t len) {
  switch (static_cast<MessageTemplate>(template_id)) {
  case MessageTemplate::OrderUpdate:
    if (len < sizeof(OrderUpdate)) {
      std::cerr << "Incomplete OrderUpdate\n";
      return std::nullopt;
    }
    return ParsedMessage{read_struct<OrderUpdate>(payload)};

  case MessageTemplate::OrderExecution:
    if (len < sizeof(OrderExecution)) {
      std::cerr << "Incomplete OrderExecution\n";
      return std::nullopt;
    }
    return ParsedMessage{read_struct<OrderExecution>(payload)};

  case MessageTemplate::OrderBookSnapshot: {
    if (len < sizeof(OrderBookSnapshotHeader)) {
      std::cerr << "Incomplete OrderBookSnapshot header\n";
      return std::nullopt;
    }

    OrderBookSnapshot snapshot;
    const auto header = read_struct<OrderBookSnapshotHeader>(payload);
    snapshot.security_id = header.security_id;
    snapshot.last_msg_seq_num_processed = header.last_msg_seq_num_processed;
    snapshot.rpt_seq = header.rpt_seq;
    snapshot.trading_session_id = header.trading_session_id;

    const uint8_t *ptr = payload + sizeof(OrderBookSnapshotHeader);
    if (ptr + sizeof(RepeatingGroupHeader) > payload + len) {
      std::cerr << "Missing repeating group header\n";
      return std::nullopt;
    }

    const auto group_header = read_struct<RepeatingGroupHeader>(ptr);
    ptr += sizeof(group_header);

    for (size_t i = 0; i < group_header.numInGroup; ++i) {
      if (ptr + group_header.blockLength > payload + len) {
        std::cerr << "Entry overflows message\n";
        return std::nullopt;
      }
      snapshot.entries.push_back(read_struct<OrderBookEntry>(ptr));
      ptr += group_header.blockLength;
    }

    return ParsedMessage{snapshot};
  }

  default:
    std::cerr << "Unknown template ID: " << template_id << "\n";
    return std::nullopt;
  }
}

} // namespace simba
