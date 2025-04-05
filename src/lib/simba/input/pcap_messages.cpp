#include "simba/input/pcap_messages.h"

#include <iomanip>

namespace simba {

std::ostream &operator<<(std::ostream &os, const PcapPacketHeader &hdr) {
  os << "PcapPacketHeader { "
     << "ts_sec: " << hdr.ts_sec << ", "
     << "ts_usec: " << std::setw(6) << std::setfill('0') << hdr.ts_usec << ", "
     << "incl_len: " << hdr.incl_len << ", "
     << "orig_len: " << hdr.orig_len << " }";
  return os;
}

} // namespace simba