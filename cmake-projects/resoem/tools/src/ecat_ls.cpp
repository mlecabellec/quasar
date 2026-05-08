#include "OptionParser.h"
#include "resoem/Enumerator.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/EtherCATTypes.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>

using namespace resoem;

int main(int argc, char* argv[]) {
    optparse::OptionParser parser = optparse::OptionParser().description("Quasar EtherCAT Topology Discovery Tool");

    parser.add_option("-i", "--interface").dest("interface").set_default("eth0").help("Network interface (default: eth0)");
    parser.add_option("-v", "--verbose").action("store_true").dest("verbose").help("Enable verbose output");

    optparse::Values options = parser.parse_args(argc, argv);
    std::vector<std::string> args = parser.args();

    std::string iface = options["interface"];
    bool verbose = options.is_set("verbose");

    try {
        std::cout << "Scanning EtherCAT bus on " << iface << "..." << std::endl;
        RawSocket socket(iface);
        Enumerator enumerator(socket);

        if (verbose) {
            enumerator.set_verbose(1);
        }

        Result<size_t> result = enumerator.enumerate();
        if (!result) {
            std::cerr << "Enumeration failed." << std::endl;
            return 1;
        }
        int count = static_cast<int>(result.value());
        std::cout << "Found " << count << " slaves.\n" << std::endl;

        std::cout << std::left << std::setfill(' ')
                  << std::setw(6)  << "Index"
                  << std::setw(10) << "Address"
                  << std::setw(10) << "Alias"
                  << std::setw(30) << "Name"
                  << std::setw(15) << "State"
                  << "Ports (A B C D)" << std::endl;
        std::cout << std::string(85, '-') << std::endl;

        int i = 0;
        const std::vector<SlaveInfo>& slaves = enumerator.slaves();
        for (std::vector<SlaveInfo>::const_iterator it = slaves.begin(); it != slaves.end(); ++it) {
            const SlaveInfo& slave = *it;
            int wkc = 0;
            uint16_t al_status = enumerator.read_register_fprd<uint16_t>(slave.configured_address, regs::AL_STATUS, wkc);
            std::string state_str = "UNKNOWN";
            if (wkc > 0) {
                uint16_t state = al_status & 0x0F;
                switch(state) {
                    case states::INIT:    state_str = "INIT"; break;
                    case states::PRE_OP:  state_str = "PRE-OP"; break;
                    case states::BOOT:    state_str = "BOOT"; break;
                    case states::SAFE_OP: state_str = "SAFE-OP"; break;
                    case states::OP:      state_str = "OP"; break;
                    default:              state_str = "ERROR"; break;
                }
            }

            // Port status logic
            uint16_t dl_status = enumerator.read_register_fprd<uint16_t>(slave.configured_address, regs::DL_STATUS, wkc);
            std::function<std::string(int)> get_port_link = [&](int bit) -> std::string {
                if (wkc == 0) return "?";
                return ((dl_status & (1 << bit)) != 0) ? "L" : ".";
            };

            std::cout << std::left
                      << std::setw(6)  << i
                      << "0x" << std::hex << std::setw(8) << slave.configured_address << std::dec
                      << "0x" << std::hex << std::setw(8) << slave.alias_address << std::dec
                      << std::setw(30) << slave.name
                      << std::setw(15) << state_str
                      << get_port_link(4) << " " << get_port_link(6) << " " << get_port_link(8) << " " << get_port_link(10)
                      << std::endl;
            i++;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
