# SimbaParser

**SimbaParser** is a modern C++20 parser for the MOEX SPECTRA SIMBA market data protocol.

It provides a fast, binary-safe parser for SIMBA messages, supporting both incremental and snapshot streams.  
The library is suitable for use in trading tools, data ingestion pipelines, and market data analysis applications.

A command-line tool, `tickplayer`, is included as a reference implementation and utility.  
It uses the SimbaParser library to read and decode `.pcap` files containing captured SIMBA market data, outputting newline-delimited JSON messages.


---

## Features

- Binary-safe decoding of SIMBA messages
- Handles fragmentation & reassembly:
  - Snapshot messages with start/mid/end fragments
  - Incremental messages with LastFragment logic
- Supports message types:
  - `OrderUpdate` (template ID 15)
  - `OrderExecution` (template ID 16)
  - `OrderBookSnapshot` (template ID 17) with repeating groups
- Outputs compact JSON per message
- Extensible, modular architecture

---

## Builing environment

- C++20-compatible compiler (GCC ≥ 10, Clang ≥ 12)
- CMake ≥ 3.20
- [Ninja](https://ninja-build.org/) build system

---

## Build Instructions

```bash
git clone https://github.com/Zenikolas/SimbaParser.git
cd SimbaParser
mkdir build && cd build
cmake -G Ninja ..
ninja