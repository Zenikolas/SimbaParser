#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace simba {

struct SimbaPayloadView {
  const uint8_t *data = nullptr;
  size_t length = 0;

  bool valid() const { return data != nullptr && length > 0; }
};

inline std::optional<SimbaPayloadView>
extract_simba_payload(const uint8_t *packet_data, size_t packet_size);

} // namespace simba
