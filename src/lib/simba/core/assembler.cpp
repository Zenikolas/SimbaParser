#include "simba/core/assembler.h"

namespace simba {

void SimbaAssembler::feed(const uint8_t *data, size_t len) {
    buffer_.insert(buffer_.end(), data, data + len);
}

} // namespace simba