#pragma once
#include <cstdint>

namespace simba {

#pragma pack(push, 1)
struct Decimal5 {
    int64_t mantissa;
    double to_double() const { return mantissa / 100000.0; }
};
#pragma pack(pop)

enum class MDEntryType : char {
    Bid = '0',
    Offer = '1',
    EmptyBook = 'J',
};

enum class MDUpdateAction : uint8_t {
    New = 0,
    Change = 1,
    Delete = 2,
};

} // namespace simba
