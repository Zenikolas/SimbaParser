# Find GoogleTest installed on the system
find_package(GTest REQUIRED)

add_executable(test_parser test_decoder.cpp test_net_parser.cpp test_pcap_reader.cpp
                           test_simba_parser.cpp)

target_link_libraries(test_parser PRIVATE GTest::gtest_main simba::core
                                          simba::format simba::input simba::net)

include(GoogleTest)
gtest_discover_tests(test_parser)
