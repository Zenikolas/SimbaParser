#include "simba/net/net_frame_parser.h"

#include "simba/core/read_struct.h"

#include <cstring>
#include <iostream>
#include <netinet/in.h>

namespace simba {
namespace {

#pragma pack(push, 1)
struct EthernetHeader {
  uint8_t dst_mac[6];
  uint8_t src_mac[6];
  uint16_t ethertype;
};

struct IPv4Header {
  uint8_t version_ihl;
  uint8_t dscp_ecn;
  uint16_t total_length;
  uint16_t identification;
  uint16_t flags_fragment_offset;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t header_checksum;
  uint32_t src_ip;
  uint32_t dst_ip;

  uint8_t ihl() const { return version_ihl & 0x0F; }
  size_t header_size() const { return ihl() * 4; }
};
#pragma pack(pop)

} // namespace

std::optional<SimbaPayloadView> extract_simba_payload(const uint8_t *data,
                                                      size_t len) {
  if (len < sizeof(EthernetHeader))
    return std::nullopt;

  EthernetHeader eth = read_struct<EthernetHeader>(data);
  if (ntohs(eth.ethertype) != 0x0800) {
    std::cerr << "Not an IPv4 packet (ethertype = 0x" << std::hex
              << ntohs(eth.ethertype) << ")\n";
    return std::nullopt;
  }

  const uint8_t *ip_start = data + sizeof(EthernetHeader);
  IPv4Header ip = read_struct<IPv4Header>(ip_start);

  if ((ip.version_ihl >> 4) != 4 || ip.ihl() < 5) {
    std::cerr << "Invalid IPv4 header\n";
    return std::nullopt;
  }

  size_t ip_len = ip.header_size();
  size_t total_offset = sizeof(EthernetHeader) + ip_len + 8; // 8 for UDP

  if (total_offset >= len) {
    std::cerr << "Header lengths exceed packet size\n";
    return std::nullopt;
  }

  SimbaPayloadView view;
  view.data = data + total_offset;
  view.length = len - total_offset;
  return view;
}

} // namespace simba