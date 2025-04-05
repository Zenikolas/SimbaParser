add_library(simba_core assembler.cpp decoder.cpp simba_parser.cpp)

add_library(simba::core ALIAS simba_core)

target_include_directories(
  simba_core PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
