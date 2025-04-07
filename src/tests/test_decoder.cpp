#include <cstring>
#include <gtest/gtest.h>
#include <vector>

#include "simba/core/decoder.h"
#include "simba/core/market_data.h"

namespace {

using namespace simba;

TEST(Decoder, OrderUpdateSingleMessage) {
  OrderUpdate msg{.md_entry_id = 42,
                  .md_entry_px = {1234500}, // 123.45
                  .md_entry_size = 10,
                  .md_flags = 0,
                  .md_flags2 = 0,
                  .security_id = 1,
                  .rpt_seq = 100,
                  .update_action = 0,
                  .entry_type = '0'};

  std::vector<uint8_t> buffer(sizeof(OrderUpdate));
  std::memcpy(buffer.data(), &msg, sizeof(OrderUpdate));

  auto result =
      decode_message(static_cast<uint16_t>(MessageTemplate::OrderUpdate),
                     buffer.data(), buffer.size());

  ASSERT_TRUE(result.has_value());

  auto &parsed = std::get<OrderUpdate>(*result);

  EXPECT_EQ(parsed.md_entry_id, 42);
  EXPECT_EQ(parsed.md_entry_px.mantissa, 1234500);
  EXPECT_EQ(parsed.md_entry_size, 10);
  EXPECT_EQ(parsed.security_id, 1);
  EXPECT_EQ(parsed.rpt_seq, 100);
  EXPECT_EQ(parsed.entry_type, '0');
}

TEST(Decoder, OrderExecutionSingleMessage) {
  OrderExecution exec{.md_entry_id = 123,
                      .md_entry_px = {2345600},
                      .md_entry_size = {10},
                      .last_px = {2345600},
                      .last_qty = 5,
                      .trade_id = 9999,
                      .md_flags = 0,
                      .md_flags2 = 0,
                      .security_id = 101,
                      .rpt_seq = 22,
                      .update_action =
                          static_cast<uint8_t>(MDUpdateAction::Change),
                      .entry_type = '1'};

  std::vector<uint8_t> buffer(sizeof(OrderExecution));
  std::memcpy(buffer.data(), &exec, sizeof(exec));

  auto result =
      decode_message(static_cast<uint16_t>(MessageTemplate::OrderExecution),
                     buffer.data(), buffer.size());
  ASSERT_TRUE(result.has_value());

  const auto &parsed = std::get<OrderExecution>(*result);

  EXPECT_EQ(parsed.md_entry_id, 123);
  EXPECT_EQ(parsed.md_entry_px.mantissa, 2345600);
  EXPECT_EQ(parsed.md_entry_size.value, 10);
  EXPECT_EQ(parsed.last_px.mantissa, 2345600);
  EXPECT_EQ(parsed.last_qty, 5);
  EXPECT_EQ(parsed.trade_id, 9999);
  EXPECT_EQ(parsed.security_id, 101);
  EXPECT_EQ(parsed.rpt_seq, 22);
  EXPECT_EQ(parsed.entry_type, '1');
}

TEST(Decoder, OrderBookSnapshotSingleEntry) {
  OrderBookSnapshotHeader header{.security_id = 1001,
                                 .last_msg_seq_num_processed = 1234,
                                 .rpt_seq = 5678,
                                 .trading_session_id = 42};

  RepeatingGroupHeader group_header{.blockLength = sizeof(OrderBookEntry),
                                    .numInGroup = 1};

  OrderBookEntry entry{.md_entry_id = {42},
                       .transact_time = 1234567890,
                       .md_entry_px = {1234500},
                       .md_entry_size = {10},
                       .trade_id = {9999},
                       .md_flags = 0,
                       .md_flags2 = 0,
                       .entry_type = '0'};

  std::vector<uint8_t> buffer(sizeof(header) + sizeof(group_header) +
                              sizeof(entry));

  uint8_t *ptr = buffer.data();
  std::memcpy(ptr, &header, sizeof(header));
  ptr += sizeof(header);
  std::memcpy(ptr, &group_header, sizeof(group_header));
  ptr += sizeof(group_header);
  std::memcpy(ptr, &entry, sizeof(entry));

  auto result =
      decode_message(static_cast<uint16_t>(MessageTemplate::OrderBookSnapshot),
                     buffer.data(), buffer.size());
  ASSERT_TRUE(result.has_value());

  const auto &snapshot = std::get<OrderBookSnapshot>(*result);
  ASSERT_EQ(snapshot.entries.size(), 1);
  EXPECT_EQ(snapshot.entries[0].md_entry_id.value, 42);
}

TEST(Decoder, OrderBookSnapshotMultipleEntries) {
  constexpr int num_entries = 3;

  OrderBookSnapshotHeader header{.security_id = 2002,
                                 .last_msg_seq_num_processed = 5678,
                                 .rpt_seq = 9000,
                                 .trading_session_id = 99};

  RepeatingGroupHeader group_header{.blockLength = sizeof(OrderBookEntry),
                                    .numInGroup = num_entries};

  std::vector<OrderBookEntry> entries;
  for (int i = 0; i < num_entries; ++i) {
    entries.push_back(
        OrderBookEntry{.md_entry_id = {100 + i},
                       .transact_time = static_cast<uint64_t>(1000000 + i),
                       .md_entry_px = {100000 + i * 10},
                       .md_entry_size = {5 + i},
                       .trade_id = {5000 + i},
                       .md_flags = 0,
                       .md_flags2 = 0,
                       .entry_type = '0'});
  }

  size_t total_size = sizeof(header) + sizeof(group_header) +
                      entries.size() * sizeof(OrderBookEntry);

  std::vector<uint8_t> buffer(total_size);

  uint8_t *ptr = buffer.data();
  std::memcpy(ptr, &header, sizeof(header));
  ptr += sizeof(header);
  std::memcpy(ptr, &group_header, sizeof(group_header));
  ptr += sizeof(group_header);

  for (const auto &entry : entries) {
    std::memcpy(ptr, &entry, sizeof(entry));
    ptr += sizeof(entry);
  }

  auto result =
      decode_message(static_cast<uint16_t>(MessageTemplate::OrderBookSnapshot),
                     buffer.data(), buffer.size());
  ASSERT_TRUE(result.has_value());

  const auto &snapshot = std::get<OrderBookSnapshot>(*result);
  ASSERT_EQ(snapshot.entries.size(), num_entries);

  for (int i = 0; i < num_entries; ++i) {
    EXPECT_EQ(snapshot.entries[i].md_entry_id.value, 100 + i);
    EXPECT_EQ(snapshot.entries[i].md_entry_size.value, 5 + i);
    EXPECT_EQ(snapshot.entries[i].md_entry_px.mantissa, 100000 + i * 10);
  }
}

} // namespace
