#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace simba {

class SimbaAssembler {
public:
  void feed(const uint8_t *data, size_t len);

  // Get the reassembled payload (valid until next feed)
  inline std::span<const uint8_t> get_payload() const;

  inline void clear();

private:
  std::vector<uint8_t> buffer_;
};

void SimbaAssembler::clear() { buffer_.clear(); }

std::span<const uint8_t> SimbaAssembler::get_payload() const {
  return std::span(buffer_.data(), buffer_.size());
}

} // namespace simba