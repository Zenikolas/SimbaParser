# Find GoogleTest installed on the system
find_package(GTest REQUIRED)

add_executable(test_parser test_simba_parser.cpp test_pcap_reader.cpp)

target_link_libraries(test_parser PRIVATE GTest::gtest_main simba::core)

include(GoogleTest)
gtest_discover_tests(test_parser)
