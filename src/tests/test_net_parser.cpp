#include "simba/net/net_frame_parser.h"

#include <vector>

#include <gtest/gtest.h>

TEST(NetParser, ExtractSimbaPayloadIPv4) {
  // Ethernet header: 14 bytes
  // IPv4 header: 20 bytes
  // UDP header: 8 bytes
  // SIMBA payload: 5 bytes ("SIMBA")
  constexpr size_t payload_offset = 14 + 20 + 8;
  std::vector<uint8_t> raw_packet(payload_offset + 5, 0);

  // Set Ethernet Ethertype = IPv4 (0x0800)
  raw_packet[12] = 0x08;
  raw_packet[13] = 0x00;

  // Set IP header version + IHL = 0x45 (IPv4 + 5 × 4 = 20 bytes)
  raw_packet[14] = 0x45;

  // Set IP protocol = UDP (0x11)
  raw_packet[23] = 0x11;

  // Set fake SIMBA payload
  raw_packet[payload_offset + 0] = 'S';
  raw_packet[payload_offset + 1] = 'I';
  raw_packet[payload_offset + 2] = 'M';
  raw_packet[payload_offset + 3] = 'B';
  raw_packet[payload_offset + 4] = 'A';

  auto view =
      simba::extract_simba_payload(raw_packet.data(), raw_packet.size());

  ASSERT_TRUE(view.has_value());
  EXPECT_EQ(view->length, 5);
  EXPECT_EQ(view->data[0], 'S');
  EXPECT_EQ(view->data[4], 'A');
}
