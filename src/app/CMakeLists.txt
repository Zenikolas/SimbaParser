add_executable(tickplayer tickplayer.cpp)
target_link_libraries(tickplayer PRIVATE simba::core pcap::core)
