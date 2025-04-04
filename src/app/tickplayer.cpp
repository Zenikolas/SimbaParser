#include <iostream>
#include <string>
#include <string_view>
#include <filesystem>
#include <optional>

#include "pcap_reader.h"

struct CmdOptions {
    std::string pcap_file;
};

std::optional<CmdOptions> parse_args(int argc, char* argv[]) {
    CmdOptions options;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if ((arg == "--file" || arg == "-f") && i + 1 < argc) {
            options.pcap_file = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: tickplayer --file <capture.pcap>\n";
            return std::nullopt;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return std::nullopt;
        }
    }

    if (options.pcap_file.empty()) {
        std::cerr << "Error: PCAP file not specified. Use --file <filename>\n";
        return std::nullopt;
    }

    return options;
}


int main(int argc, char* argv[]) {
    auto parsed = parse_args(argc, argv);
    if (!parsed) {
        return 1;
    }

    const auto& opts = *parsed;
    std::cout << "Reading PCAP from: " << opts.pcap_file << "\n";

    pcap::Reader reader(opts.pcap_file);

    if (!reader.is_open() || reader.read_global_header()) {
        return 1;
    }

    while (true) {
        pcap::PcapPacketHeader header;
        std::vector<char> msg;
        if (!reader.read_next_packet(header, msg)) {
            std::cout << "Reached end of file, exiting\n";
            break;
        }

        std::cout << header << msg.size() << "\n";
    }



    return 0;
}
