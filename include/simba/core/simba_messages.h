#pragma once

#include <string>
#include <vector>

#include "simba/core/market_data.h"

namespace simba {

enum class MDEntryType : char {
  Bid = '0',
  Offer = '1',
  EmptyBook = 'J',
};

enum class MDUpdateAction : uint8_t {
  New = 0,
  Change = 1,
  Delete = 2,
};

enum class MessageTemplate : uint16_t {
  OrderUpdate = 15,
  OrderExecution = 16,
  OrderBookSnapshot = 17,
};

#pragma pack(push, 1)

struct MarketDataPacketHeader {
  uint32_t msg_seq_num;
  uint16_t msg_size;
  uint16_t msg_flags;
  uint64_t sending_time;
};

struct IncrementalPacketHeader {
  uint64_t transact_time;
  uint32_t session_id;
};

struct SBEHeader {
  uint16_t block_length;
  uint16_t template_id;
  uint16_t schema_id;
  uint16_t version;
};

struct OrderUpdate {
  int64_t md_entry_id;
  Decimal5 md_entry_px;
  int64_t md_entry_size;
  uint64_t md_flags;
  uint64_t md_flags2;
  int32_t security_id;
  uint32_t rpt_seq;
  uint8_t update_action;
  char entry_type; // '0'=Bid, '1'=Ask
};

struct OrderExecution {
  int64_t md_entry_id;
  Decimal5NULL md_entry_px;
  Int64NULL md_entry_size;
  Decimal5 last_px;
  int64_t last_qty;
  int64_t trade_id;
  uint64_t md_flags;
  uint64_t md_flags2;
  int32_t security_id;
  uint32_t rpt_seq;
  uint8_t update_action;
  char entry_type;
};

struct OrderBookEntry {
  Int64NULL md_entry_id;
  uint64_t transact_time;
  Decimal5NULL md_entry_px;
  Int64NULL md_entry_size;
  Int64NULL trade_id;
  uint64_t md_flags;
  uint64_t md_flags2;
  char entry_type;
};

struct OrderBookSnapshotHeader {
  int32_t security_id;
  uint32_t last_msg_seq_num_processed;
  uint32_t rpt_seq;
  uint32_t trading_session_id;
};

struct RepeatingGroupHeader {
  uint16_t blockLength;
  uint8_t numInGroup;
};
#pragma pack(pop)

struct OrderBookSnapshot {
  int32_t security_id;
  uint32_t last_msg_seq_num_processed;
  uint32_t rpt_seq;
  uint32_t trading_session_id;
  std::vector<OrderBookEntry> entries;
};

} // namespace simba
