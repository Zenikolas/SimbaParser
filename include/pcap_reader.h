#pragma once
#include <fstream>
#include <string>
#include <vector>

#include "pcap_messages.h"

namespace pcap {

std::ostream& operator<<(std::ostream& os, const PcapPacketHeader& hdr);

class Reader {
public:
  explicit Reader(const std::string &filepath)
      : file(filepath, std::ios::binary) {}

  bool is_open() const { return file.is_open(); }

  bool read_global_header() {
    if (!file.read(reinterpret_cast<char *>(&global_header),
                   sizeof(global_header))) {
      std::cerr << "Failed to read global header\n";
      return false;
    }

    // Check magic number for endianness (0xa1b2c3d4 for LE)
    if (global_header.magic_number != 0xa1b2c3d4) {
      std::cerr << "Unsupported format or endian mismatch: " << std::hex
                << global_header.magic_number << "\n";
      return false;
    }

    std::cout << "PCAP version " << global_header.version_major << "."
              << global_header.version_minor << "\n";
    return true;
  }

  bool read_next_packet(PcapPacketHeader &pkt_hdr, std::vector<char> &data) {
    if (!file.read(reinterpret_cast<char *>(&pkt_hdr), sizeof(pkt_hdr))) {
      return false;
    }

    if (static_cast<size_t>(file.gcount()) < sizeof(pkt_hdr)) {
      std::cerr << "Incomplete packet header\n";
      return false;
    }

    data.resize(pkt_hdr.incl_len);
    if (!file.read(data.data(), pkt_hdr.incl_len)) {
      std::cerr << "Failed to read packet data\n";
      return false;
    }

    return true;
  }

private:
  std::ifstream file;
  PcapGlobalHeader global_header;
};

} // namespace pcap