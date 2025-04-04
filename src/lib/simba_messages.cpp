#include "simba_messages.h"

#include <sstream>

namespace simba {

std::string to_str(MDEntryType type) {
    switch (type) {
        case MDEntryType::Bid: return "Bid";
        case MDEntryType::Offer: return "Offer";
        case MDEntryType::EmptyBook: return "EmptyBook";
        default: return "Unknown";
    }
}

std::string to_str(MDUpdateAction action) {
    switch (action) {
        case MDUpdateAction::New: return "New";
        case MDUpdateAction::Change: return "Change";
        case MDUpdateAction::Delete: return "Delete";
        default: return "Unknown";
    }
}

std::string OrderUpdate::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"transact_time\":" << transact_time << ","
        << "\"security_id\":" << security_id << ","
        << "\"price\":" << price.to_double() << ","
        << "\"size\":" << size << ","
        << "\"entry_id\":" << entry_id << ","
        << "\"md_flags\":" << md_flags << ","
        << "\"md_flags2\":" << md_flags2 << ","
        << "\"action\":\"" << to_str(action) << "\","
        << "\"entry_type\":\"" << to_str(entry_type) << "\""
        << "}";
    return oss.str();
}

} // namespace simba
