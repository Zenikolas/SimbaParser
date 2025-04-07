# SimbaParser

**SimbaParser** is a C++20-based parser library for the MOEX SIMBA SPECTRA market data protocol. 
tickplaye is the application using SIMBA parser which processes raw binary UDP payloads extracted from `.pcap` files and outputs structured newline-delimited JSON.

## Features

- Binary-safe decoding of SIMBA protocol messages
- Full support for **incremental** and **snapshot** message types:
  - `OrderUpdate` (msg ID 15)
  - `OrderExecution` (msg ID 16)
  - `OrderBookSnapshot` (msg ID 17), including repeating groups

## Further improvements

- Add parsing support for more SIMBA messages
- Add app listeing on the socket to capture mesages from wire
- Add proper logging framework
- Improve test code coverage
