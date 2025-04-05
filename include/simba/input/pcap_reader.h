#pragma once
#include <fstream>
#include <string>
#include <vector>

#include "pcap_messages.h"

namespace simba {

class Reader {
public:
  explicit Reader(const std::string &filepath)
      : file(filepath, std::ios::binary) {}

  bool is_open() const { return file.is_open(); }

  bool read_global_header();

  bool read_next_packet(PcapPacketHeader &pkt_hdr, std::vector<char> &data);

private:
  std::ifstream file;
  PcapGlobalHeader global_header;
};

} // namespace simba