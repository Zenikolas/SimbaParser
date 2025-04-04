add_library(simba_core simba_messages.cpp simba_parser.cpp)

add_library(pcap_core pcap_messages.cpp)

add_library(simba::core ALIAS simba_core)

add_library(pcap::core ALIAS pcap_core)

target_include_directories(
  simba_core PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

  target_include_directories(
    pcap_core PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
