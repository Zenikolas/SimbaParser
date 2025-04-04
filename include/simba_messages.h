#pragma once

#include <string>

#include "simba_types.h"

namespace simba {

#pragma pack(push, 1)
struct OrderUpdate {
    uint64_t transact_time;
    int32_t security_id;
    Decimal5 price;
    int64_t size;
    uint64_t entry_id;
    uint32_t md_flags;
    uint32_t md_flags2;
    MDUpdateAction action;
    MDEntryType entry_type;

    std::string to_json() const;
};
#pragma pack(pop)

} // namespace simba
