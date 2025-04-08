#include "simba/core/simba_parser.h"

#include "simba/core/simba_messages.h"

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
};

auto make_incremental_body(const std::vector<OrderUpdate> &updates) {
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
};

auto make_snapshot_fragment(bool start, bool end,
                            const std::vector<uint8_t> &payload) {
  uint16_t flags = 0;
  if (start)
    flags |= 0x02;
  if (end)
    flags |= 0x04;

  return make_packet(flags, payload);
};
} // namespace

TEST(SimbaParser, IncrementalFragmentation) {
  std::vector<ParsedMessage> messages;

  SimbaParser parser(
      [&messages](const ParsedMessage &msg) { messages.push_back(msg); });

  // Simulate an OrderUpdate message across 2 fragments
  SBEHeader sbe{.block_length = sizeof(OrderUpdate),
                .template_id =
                    static_cast<uint16_t>(MessageTemplate::OrderUpdate),
                .schema_id = 1,
                .version = 1};

  OrderUpdate msg{.md_entry_id = 555,
                  .md_entry_px = {1000000},
                  .md_entry_size = 10,
                  .md_flags = 0,
                  .md_flags2 = 0,
                  .security_id = 42,
                  .rpt_seq = 1,
                  .update_action = 0,
                  .entry_type = '0'};

  // Simulated UDP payload
  std::vector<uint8_t> simba_payload(sizeof(sbe) + sizeof(msg));
  std::memcpy(simba_payload.data(), &sbe, sizeof(sbe));
  std::memcpy(simba_payload.data() + sizeof(sbe), &msg, sizeof(msg));

  // Split into two fragments
  size_t split = 10; // arbitrary split offset
  std::vector<uint8_t> frag1(simba_payload.begin(),
                             simba_payload.begin() + split);
  std::vector<uint8_t> frag2(simba_payload.begin() + split,
                             simba_payload.end());

  // Create fake incremental packets with MarketDataPacketHeader
  auto make_header = [](bool last) -> MarketDataPacketHeader {
    return {.msg_seq_num = 1,
            .msg_size = 0,
            .msg_flags = static_cast<uint16_t>(
                0x08 | (last ? 0x01 : 0x00)), // incremental, last fragment flag
            .sending_time = 0};
  };

  auto make_incremental_header = []() -> IncrementalPacketHeader {
    return {.transact_time = 0, .session_id = 0};
  };

  auto build_packet = [](const MarketDataPacketHeader &h,
                         const IncrementalPacketHeader &inc,
                         const std::vector<uint8_t> &body) {
    std::vector<uint8_t> packet(sizeof(h) + sizeof(inc) + body.size());
    uint8_t *p = packet.data();
    std::memcpy(p, &h, sizeof(h));
    std::memcpy(p + sizeof(h), &inc, sizeof(inc));
    std::memcpy(p + sizeof(h) + sizeof(inc), body.data(), body.size());
    return packet;
  };

  // Feed fragment 1
  auto packet1 =
      build_packet(make_header(false), make_incremental_header(), frag1);
  parser.feed(packet1.data(), packet1.size());

  // Feed fragment 2
  auto packet2 =
      build_packet(make_header(true), make_incremental_header(), frag2);
  parser.feed(packet2.data(), packet2.size());

  ASSERT_EQ(messages.size(), 1);
  auto &parsed = std::get<OrderUpdate>(messages[0]);
  EXPECT_EQ(parsed.md_entry_id, 555);
}

TEST(SimbaParser, IncrementalAndSnapshotMixedTest) {
  std::vector<ParsedMessage> messages;

  SimbaParser parser(
      [&](const ParsedMessage &msg) { messages.push_back(msg); });

  // Packet 1: Incremental, 1 OrderUpdate
  OrderUpdate upd1 = {.md_entry_id = 1,
                      .md_entry_px = {100000},
                      .md_entry_size = 10,
                      .md_flags = 0,
                      .md_flags2 = 0,
                      .security_id = 1,
                      .rpt_seq = 1,
                      .update_action = 0,
                      .entry_type = '0'};
  auto p1 = make_packet(0x09, make_incremental_body({upd1}));
  parser.feed(p1.data(), p1.size());

  // Packet 2: Incremental, 2 OrderUpdates
  OrderUpdate upd2 = upd1;
  upd2.md_entry_id = 2;
  OrderUpdate upd3 = upd1;
  upd3.md_entry_id = 3;
  auto p2 = make_packet(0x09, make_incremental_body({upd2, upd3}));
  parser.feed(p2.data(), p2.size());

  // Snapshot parts
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

  // Fragment 1: Start
  std::vector<uint8_t> snap1(sizeof(SBEHeader) + sizeof(snap_hdr));
  SBEHeader sbe_snap{sizeof(snap_hdr), 17, 1, 1};
  std::memcpy(snap1.data(), &sbe_snap, sizeof(sbe_snap));
  std::memcpy(snap1.data() + sizeof(sbe_snap), &snap_hdr, sizeof(snap_hdr));
  auto p3 = make_snapshot_fragment(true, false, snap1);
  parser.feed(p3.data(), p3.size());

  // Fragment 2: Middle
  std::vector<uint8_t> snap2(sizeof(group_hdr));
  std::memcpy(snap2.data(), &group_hdr, sizeof(group_hdr));
  auto p4 = make_snapshot_fragment(false, false, snap2);
  parser.feed(p4.data(), p4.size());

  // Fragment 3: End (2 entries)
  std::vector<uint8_t> snap3(sizeof(e1) + sizeof(e2));
  std::memcpy(snap3.data(), &e1, sizeof(e1));
  std::memcpy(snap3.data() + sizeof(e1), &e2, sizeof(e2));
  auto p5 = make_snapshot_fragment(false, true, snap3);
  parser.feed(p5.data(), p5.size());

  ASSERT_EQ(messages.size(), 4); // 3 incremental + 1 snapshot

  EXPECT_TRUE(std::holds_alternative<OrderUpdate>(messages[0]));
  EXPECT_TRUE(std::holds_alternative<OrderUpdate>(messages[1]));
  EXPECT_TRUE(std::holds_alternative<OrderUpdate>(messages[2]));
  EXPECT_TRUE(std::holds_alternative<OrderBookSnapshot>(messages[3]));

  const auto &snapshot = std::get<OrderBookSnapshot>(messages[3]);
  ASSERT_EQ(snapshot.entries.size(), 2);
  EXPECT_EQ(snapshot.entries[0].md_entry_id.value, 111);
  EXPECT_EQ(snapshot.entries[1].md_entry_id.value, 222);
}
} // namespace simba
