add_library(simba_net net_frame_parser.cpp)

add_library(simba::net ALIAS simba_net)

target_include_directories(
  simba_net PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
