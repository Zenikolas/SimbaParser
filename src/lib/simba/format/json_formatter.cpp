#include "simba/format/json_formatter.h"

#include <sstream>
#include <variant>

#include "simba/core/market_data.h"

namespace simba {

std::string JsonFormatter::format(const OrderUpdate &msg) {
  std::ostringstream oss;
  oss << "{";
  oss << "\"type\":\"OrderUpdate\",";
  oss << "\"mdEntryID\":" << msg.md_entry_id << ",";
  oss << "\"mdEntryPx\":" << msg.md_entry_px.to_double() << ",";
  oss << "\"mdEntrySize\":" << msg.md_entry_size << ",";
  oss << "\"mdFlags\":" << msg.md_flags << ",";
  oss << "\"mdFlags2\":" << msg.md_flags2 << ",";
  oss << "\"securityID\":" << msg.security_id << ",";
  oss << "\"rptSeq\":" << msg.rpt_seq << ",";
  oss << "\"updateAction\":" << static_cast<int>(msg.update_action) << ",";
  oss << "\"entryType\":\"" << msg.entry_type << "\"";
  oss << "}";
  return oss.str();
}

std::string JsonFormatter::format(const OrderExecution &msg) {
  std::ostringstream oss;
  oss << "{";
  oss << "\"type\":\"OrderExecution\",";
  oss << "\"mdEntryID\":" << msg.md_entry_id.to_string() << ",";
  oss << "\"mdEntryPx\":" << msg.md_entry_px.to_string() << ",";
  oss << "\"mdEntrySize\":" << msg.md_entry_size.to_string() << ",";
  oss << "\"lastPx\":" << msg.last_px.to_double() << ",";
  oss << "\"lastQty\":" << msg.last_qty << ",";
  oss << "\"tradeID\":" << msg.trade_id << ",";
  oss << "\"mdFlags\":" << msg.md_flags << ",";
  oss << "\"mdFlags2\":" << msg.md_flags2 << ",";
  oss << "\"securityID\":" << msg.security_id << ",";
  oss << "\"rptSeq\":" << msg.rpt_seq << ",";
  oss << "\"updateAction\":" << static_cast<int>(msg.update_action) << ",";
  oss << "\"entryType\":\"" << msg.entry_type << "\"";
  oss << "}";
  return oss.str();
}

std::string JsonFormatter::format(const OrderBookEntry &msg) {
  std::ostringstream oss;
  oss << "{";
  oss << "\"mdEntryID\":" << msg.md_entry_id.to_string() << ",";
  oss << "\"transactTime\":" << msg.transact_time << ",";
  oss << "\"mdEntryPx\":" << msg.md_entry_px.to_string() << ",";
  oss << "\"mdEntrySize\":" << msg.md_entry_size.to_string() << ",";
  oss << "\"tradeID\":" << msg.trade_id.to_string() << ",";
  oss << "\"mdFlags\":" << msg.md_flags << ",";
  oss << "\"mdFlags2\":" << msg.md_flags2 << ",";
  oss << "\"entryType\":\"" << msg.entry_type << "\"";
  oss << "}";
  return oss.str();
}

std::string JsonFormatter::format(const OrderBookSnapshot &msg) {
  std::ostringstream oss;
  oss << "{";
  oss << "\"type\":\"OrderBookSnapshot\",";
  oss << "\"securityID\":" << msg.security_id << ",";
  oss << "\"lastMsgSeqNumProcessed\":" << msg.last_msg_seq_num_processed << ",";
  oss << "\"rptSeq\":" << msg.rpt_seq << ",";
  oss << "\"tradingSessionID\":" << msg.trading_session_id << ",";
  oss << "\"entries\":[";
  for (size_t i = 0; i < msg.entries.size(); ++i) {
    oss << JsonFormatter::format(msg.entries[i]);
    if (i + 1 < msg.entries.size())
      oss << ",";
  }
  oss << "]";
  oss << "}";
  return oss.str();
}

std::string JsonFormatter::format(const ParsedMessage &parsedMsg) {
  return std::visit([](const auto &msg) { return JsonFormatter::format(msg); },
                    parsedMsg);
}

} // namespace simba
