add_library(simba_format json_formatter.cpp)

add_library(simba::format ALIAS simba_format)

target_include_directories(
  simba_format PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

target_link_libraries(simba_format PUBLIC simba::core)
