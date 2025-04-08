#include "simba/core/simba_messages.h"
#include "simba/core/simba_parser.h"

#include <cstring>
#include <span>
#include <vector>

#include <gtest/gtest.h>

namespace simba {
namespace {

auto make_packet(uint16_t msg_flags, const std::vector<uint8_t> &body) {
  MarketDataPacketHeader header{
      .msg_seq_num = 0,
      .msg_size =
          static_cast<uint16_t>(sizeof(MarketDataPacketHeader) + body.size()),
      .msg_flags = msg_flags,
      .sending_time = 0};

  std::vector<uint8_t> buffer(sizeof(header) + body.size());
  std::memcpy(buffer.data(), &header, sizeof(header));
  std::memcpy(buffer.data() + sizeof(header), body.data(), body.size());
  return buffer;
}

auto make_incremental_order_updates(const std::vector<OrderUpdate> &updates) {
  std::vector<uint8_t> buf;
  IncrementalPacketHeader inc_hdr{.transact_time = 0, .session_id = 0};
  buf.resize(sizeof(inc_hdr));
  std::memcpy(buf.data(), &inc_hdr, sizeof(inc_hdr));

  for (const auto &update : updates) {
    SBEHeader sbe{.block_length = sizeof(OrderUpdate),
                  .template_id =
                      static_cast<uint16_t>(MessageTemplate::OrderUpdate),
                  .schema_id = 1,
                  .version = 1};

    size_t offset = buf.size();
    buf.resize(offset + sizeof(sbe) + sizeof(OrderUpdate));
    std::memcpy(buf.data() + offset, &sbe, sizeof(sbe));
    std::memcpy(buf.data() + offset + sizeof(sbe), &update, sizeof(update));
  }

  return buf;
}

// Helper to make a snapshot fragment with appropriate flags
auto make_snapshot_fragment(bool start, bool end,
                            const std::vector<uint8_t> &payload) {
  uint16_t flags = 0;
  if (start)
    flags |= 0x02;
  if (end)
    flags |= 0x04;

  return make_packet(flags, payload);
}

} // namespace

TEST(SimbaParser, IncrementalSnapshotThenOrderExecution) {
  std::vector<ParsedMessage> messages;

  SimbaParser parser(
      [&](const ParsedMessage &msg) { messages.push_back(msg); });

  // === Incremental: 2 OrderUpdates ===
  OrderUpdate upd1 = {.md_entry_id = 10,
                      .md_entry_px = {100000},
                      .md_entry_size = 10,
                      .md_flags = 0,
                      .md_flags2 = 0,
                      .security_id = 1,
                      .rpt_seq = 1,
                      .update_action = 0,
                      .entry_type = '0'};

  OrderUpdate upd2 = upd1;
  upd2.md_entry_id = 20;

  auto incr_body = make_incremental_order_updates({upd1, upd2});
  auto incr_packet = make_packet(0x08, incr_body); // incremental
  parser.feed(incr_packet.data(), incr_packet.size());

  // === Snapshot ===
  OrderBookSnapshotHeader snap_hdr = {.security_id = 1001,
                                      .last_msg_seq_num_processed = 0,
                                      .rpt_seq = 0,
                                      .trading_session_id = 0};

  RepeatingGroupHeader group_hdr = {.blockLength = sizeof(OrderBookEntry),
                                    .numInGroup = 2};

  OrderBookEntry e1 = {.md_entry_id = {111},
                       .transact_time = 123,
                       .md_entry_px = {101000},
                       .md_entry_size = {1},
                       .trade_id = {500},
                       .md_flags = 0,
                       .md_flags2 = 0,
                       .entry_type = '0'};

  OrderBookEntry e2 = e1;
  e2.md_entry_id = {222};

  std::vector<uint8_t> snap_payload;
  SBEHeader sbe_snap{
      .block_length = sizeof(snap_hdr),
      .template_id = static_cast<uint16_t>(MessageTemplate::OrderBookSnapshot),
      .schema_id = 1,
      .version = 1};

  size_t offset = 0;
  snap_payload.resize(sizeof(sbe_snap) + sizeof(snap_hdr) + sizeof(group_hdr) +
                      sizeof(e1) + sizeof(e2));

  std::memcpy(snap_payload.data() + offset, &sbe_snap, sizeof(sbe_snap));
  offset += sizeof(sbe_snap);

  std::memcpy(snap_payload.data() + offset, &snap_hdr, sizeof(snap_hdr));
  offset += sizeof(snap_hdr);

  std::memcpy(snap_payload.data() + offset, &group_hdr, sizeof(group_hdr));
  offset += sizeof(group_hdr);

  std::memcpy(snap_payload.data() + offset, &e1, sizeof(e1));
  offset += sizeof(e1);

  std::memcpy(snap_payload.data() + offset, &e2, sizeof(e2));

  auto snapshot_packet = make_snapshot_fragment(true, true, snap_payload);
  parser.feed(snapshot_packet.data(), snapshot_packet.size());

  // === OrderExecution ===
  OrderExecution exec_msg = {.md_entry_id = 888,
                             .md_entry_px = {105000},
                             .md_entry_size = {15},
                             .last_px = {106000},
                             .last_qty = 5,
                             .trade_id = 5555,
                             .md_flags = 0,
                             .md_flags2 = 0,
                             .security_id = 1001,
                             .rpt_seq = 3,
                             .update_action = 0,
                             .entry_type = '2'};

  SBEHeader sbe_exec{.block_length = sizeof(OrderExecution),
                     .template_id =
                         static_cast<uint16_t>(MessageTemplate::OrderExecution),
                     .schema_id = 1,
                     .version = 1};

  std::vector<uint8_t> exec_payload(sizeof(sbe_exec) + sizeof(exec_msg));
  std::memcpy(exec_payload.data(), &sbe_exec, sizeof(sbe_exec));
  std::memcpy(exec_payload.data() + sizeof(sbe_exec), &exec_msg,
              sizeof(exec_msg));

  IncrementalPacketHeader exec_inc_hdr{.transact_time = 1, .session_id = 0};

  std::vector<uint8_t> exec_full_payload(sizeof(exec_inc_hdr) +
                                         exec_payload.size());
  std::memcpy(exec_full_payload.data(), &exec_inc_hdr, sizeof(exec_inc_hdr));
  std::memcpy(exec_full_payload.data() + sizeof(exec_inc_hdr),
              exec_payload.data(), exec_payload.size());

  auto exec_packet = make_packet(0x08, exec_full_payload); // incremental

  parser.feed(exec_packet.data(), exec_packet.size());

  // === Assertions ===
  ASSERT_EQ(messages.size(), 4);

  EXPECT_TRUE(std::holds_alternative<OrderUpdate>(messages[0]));
  EXPECT_TRUE(std::holds_alternative<OrderUpdate>(messages[1]));
  EXPECT_TRUE(std::holds_alternative<OrderBookSnapshot>(messages[2]));
  EXPECT_TRUE(std::holds_alternative<OrderExecution>(messages[3]));

  const auto &snap = std::get<OrderBookSnapshot>(messages[2]);
  ASSERT_EQ(snap.entries.size(), 2);
  EXPECT_EQ(snap.entries[0].md_entry_id.value, 111);
  EXPECT_EQ(snap.entries[1].md_entry_id.value, 222);

  const auto &exec = std::get<OrderExecution>(messages[3]);
  EXPECT_EQ(exec.md_entry_id, 888);
  EXPECT_EQ(exec.last_px.mantissa, 106000);
  EXPECT_EQ(exec.last_qty, 5);
  EXPECT_EQ(exec.trade_id, 5555);
  EXPECT_EQ(exec.security_id, 1001);
}

} // namespace simba
