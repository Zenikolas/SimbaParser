#include <gtest/gtest.h>


#include "pcap_reader.h"
#include "pcap_messages.h"

namespace pcap {

// Helper function to create a temporary PCAP file for testing
void create_test_pcap(const std::string &filepath, const PcapGlobalHeader &global_header,
                      const std::vector<std::pair<PcapPacketHeader, std::vector<char>>> &packets) {
    std::ofstream file(filepath, std::ios::binary);
    file.write(reinterpret_cast<const char *>(&global_header), sizeof(global_header));
    for (const auto &packet : packets) {
        file.write(reinterpret_cast<const char *>(&packet.first), sizeof(packet.first));
        file.write(packet.second.data(), packet.second.size());
    }
    file.close();
}

TEST(PcapReaderTest, FileOpenTest) {
    // Test with a non-existent file
    pcap::Reader reader("non_existent.pcap");
    EXPECT_FALSE(reader.is_open());
}

TEST(PcapReaderTest, ReadGlobalHeaderTest) {
    // Create a valid PCAP file
    const std::string test_file = "test_global_header.pcap";
    PcapGlobalHeader global_header = {0xa1b2c3d4, 2, 4, 0, 0, 65535, 1};
    create_test_pcap(test_file, global_header, {});

    pcap::Reader reader(test_file);
    ASSERT_TRUE(reader.is_open());
    EXPECT_TRUE(reader.read_global_header());

    // Clean up
    std::remove(test_file.c_str());
}

TEST(PcapReaderTest, ReadNextPacketTest) {
    // Create a valid PCAP file with one packet
    const std::string test_file = "test_packet.pcap";
    PcapGlobalHeader global_header = {0xa1b2c3d4, 2, 4, 0, 0, 65535, 1};
    PcapPacketHeader packet_header = {12345678, 0, 4, 4};
    std::vector<char> packet_data = {'t', 'e', 's', 't'};
    create_test_pcap(test_file, global_header, {{packet_header, packet_data}});

    pcap::Reader reader(test_file);
    ASSERT_TRUE(reader.is_open());
    ASSERT_TRUE(reader.read_global_header());

    PcapPacketHeader read_packet_header;
    std::vector<char> read_packet_data;
    EXPECT_TRUE(reader.read_next_packet(read_packet_header, read_packet_data));
    EXPECT_EQ(read_packet_header.incl_len, packet_header.incl_len);
    EXPECT_EQ(read_packet_data, packet_data);

    // Clean up
    std::remove(test_file.c_str());
}

} // namespace pcap