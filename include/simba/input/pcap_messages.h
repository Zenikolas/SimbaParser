#pragma once
#include <cstdint>
#include <iostream>

namespace simba {

#pragma pack(push, 1)

// --- PCAP global header (24 bytes)
struct PcapGlobalHeader {
  uint32_t magic_number; // Endian check
  uint16_t version_major;
  uint16_t version_minor;
  int32_t thiszone;
  uint32_t sigfigs;
  uint32_t snaplen;
  uint32_t network;
};

// --- PCAP packet header (16 bytes)
struct PcapPacketHeader {
  uint32_t ts_sec;
  uint32_t ts_usec;
  uint32_t incl_len;
  uint32_t orig_len;
};

#pragma pack(pop)

std::ostream &operator<<(std::ostream &os, const PcapPacketHeader &hdr);

} // namespace simba