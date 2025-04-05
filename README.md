# SimbaParser

**SimbaParser** is a C++20-based parser for the MOEX SIMBA SPECTRA market data protocol.  
It processes raw binary UDP payloads extracted from `.pcap` files and outputs structured newline-delimited JSON.  
Built for high-throughput and real-time streaming, it is modular, fast, and safe for production environments.

---

## ✅ Features

- Binary-safe decoding of SIMBA protocol messages
- Full support for **incremental** and **snapshot** message types:
  - `OrderUpdate` (msg ID 15)
  - `OrderExecution` (msg ID 16)
  - `OrderBookSnapshot` (msg ID 17), including repeating groups
- Proper handling of **message fragmentation** using the `LastFragment` flag
- Robust parsing of **SBE-encoded** messages using template IDs
- Clean separation of concerns:
  - Network parsing (Ethernet, IP, UDP)
  - SIMBA protocol logic
  - Message formatting (JSON)
- `ParsedMessage` variant for unified message handling
- Zero-exception, zero-allocation hot-path processing
- Outputs each message as a single JSON line