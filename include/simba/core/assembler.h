#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace simba {

class SimbaAssembler {
public:

  // Feed a fragment from a packet
  // Returns true if this is a complete message
  bool feed(const uint8_t *data, size_t len, bool is_fragmented);

  // Get the reassembled payload (valid until next feed)
  inline std::span<const uint8_t> get_payload() const;

  void clear();

private:
  std::vector<uint8_t> buffer_;
  bool buffering_ = false;
};

std::span<const uint8_t> SimbaAssembler::get_payload() const {
  return std::span(buffer_.data(), buffer_.size());
}

} // namespace simba