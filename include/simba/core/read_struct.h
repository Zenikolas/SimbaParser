#pragma once

namespace simba {

template <typename T>
  requires std::is_trivially_copyable_v<T>
inline T read_struct(const uint8_t *data) {
  T out;
  std::memcpy(&out, data, sizeof(T));
  return out;
}

} // namespace simba