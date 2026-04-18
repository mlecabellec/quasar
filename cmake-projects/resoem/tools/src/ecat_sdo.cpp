#include "OptionParser.h"
#include "resoem/Enumerator.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/MailboxHandler.hpp"
#include "resoem/CoEHandler.hpp"
#include "resoem/tools/Common.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace resoem;

int main(int argc, char* argv[]) {
    optparse::OptionParser parser = optparse::OptionParser().description("Quasar EtherCAT SDO Utility");

    parser.add_option("-i", "--interface").dest("interface").set_default("eth0").help("Network interface");
    parser.add_option("-s", "--slave").dest("slave").type("int").set_default(0).help("Target slave index");
    parser.add_option("-r", "--read").dest("read").help("Read SDO (index:subindex)");
    parser.add_option("-w", "--write").dest("write").help("Write SDO (index:subindex value_hex)");

    optparse::Values options = parser.parse_args(argc, argv);
    std::string iface = options["interface"];
    int slave_idx = options.get("slave");

    try {
        RawSocket socket(iface);
        Enumerator enumerator(socket);
        auto enum_res = enumerator.enumerate();
        if (!enum_res) return 1;

        if (slave_idx < 0 || slave_idx >= static_cast<int>(enumerator.slaves().size())) {
            std::cerr << "Invalid slave index." << std::endl;
            return 1;
        }

        auto& slave = const_cast<SlaveInfo&>(enumerator.slaves()[slave_idx]);
        MailboxHandler mailbox(socket);
        CoEHandler coe(mailbox);

        if (options.is_set("read")) {
            std::string target = options["read"];
            size_t colon = target.find(':');
            uint16_t index = std::stoul(target.substr(0, colon), nullptr, 16);
            uint8_t subindex = (colon != std::string::npos) ? std::stoul(target.substr(colon + 1), nullptr, 16) : 0;

            std::vector<uint8_t> buffer(1024);
            auto res = coe.sdo_read(slave, index, subindex, std::span<uint8_t>(buffer));
            if (res) {
                std::cout << "Value: " << resoem::tools::format_hex(buffer.data(), res.value()) << std::endl;
            }
        } else if (options.is_set("write")) {
            // Simple parsing for "index:subindex value"
            // For now, we take from remaining args if any
            std::vector<std::string> pos_args = parser.args();
            if (pos_args.empty()) {
                std::cerr << "Value required for write." << std::endl;
                return 1;
            }
            std::string target = options["write"];
            size_t colon = target.find(':');
            uint16_t index = std::stoul(target.substr(0, colon), nullptr, 16);
            uint8_t subindex = (colon != std::string::npos) ? std::stoul(target.substr(colon + 1), nullptr, 16) : 0;

            std::vector<uint8_t> data = resoem::tools::parse_hex(pos_args[0]);
            auto res = coe.sdo_write(slave, index, subindex, std::span<const uint8_t>(data));
            if (res) {
                std::cout << "Write Success." << std::endl;
            }
        } else {
            // Default: Browse OD
            auto list_res = coe.read_od_list(slave);
            if (list_res) {
                std::cout << "Object Dictionary for Slave " << slave_idx << ":" << std::endl;
                for (uint16_t idx : list_res.value()) {
                    auto desc = coe.read_od_description(slave, idx);
                    if (desc) {
                        std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0') << idx << ": " 
                                  << desc.value().name << std::dec << std::endl;
                    }
                }
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
