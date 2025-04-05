add_library(simba_input pcap_messages.cpp pcap_reader.cpp)

add_library(simba::input ALIAS simba_input)

target_include_directories(
  simba_input PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
