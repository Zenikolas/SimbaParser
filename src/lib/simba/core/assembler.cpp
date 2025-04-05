#include "simba/core/assembler.h"

namespace simba {

bool SimbaAssembler::feed(const uint8_t *data, size_t len, bool is_fragmented) {
  if (!is_fragmented) {
    // Single message
    buffer_.assign(data, data + len);
    buffering_ = false;
    return true;
  }

  // Fragmented: append to buffer
  buffer_.insert(buffer_.end(), data, data + len);
  buffering_ = true;

  // Return true only when the last fragment is fed
  return false;
}

} // namespace simba