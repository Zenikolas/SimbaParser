#include "simba/core/simba_parser.h"

#include "simba/core/simba_messages.h"

#include <cstring>
#include <span>
#include <vector>

#include <gtest/gtest.h>

namespace simba {

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

// TODO: add test for snapshot 3 fragments, several incremental in one message
} // namespace simba
