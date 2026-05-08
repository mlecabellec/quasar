#include "OptionParser.h"
#include "resoem/Enumerator.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/EtherCATTypes.hpp"
#include "resoem/Diagnostics.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace resoem;

void print_fmmu(Enumerator& enumerator, uint16_t addr) {
    std::cout << "\nFMMU Configuration (max 16 entries):" << std::endl;
    std::cout << std::left << std::setw(6) << "Idx" << std::setw(12) << "LogAddr" << std::setw(10) << "Len" 
              << std::setw(10) << "StartBit" << std::setw(10) << "EndBit" << std::setw(12) << "PhyAddr" << "Type" << std::endl;
    std::cout << std::string(75, '-') << std::endl;

    for (int i = 0; i < 16; ++i) {
        int wkc = 0;
        // Each FMMU is 16 bytes. Read raw.
        // We'll just read the physical start address to see if it's used.
        uint16_t phy = enumerator.read_register_fprd<uint16_t>(addr, 0x0600 + (i * 16) + 4, wkc);
        uint16_t len = enumerator.read_register_fprd<uint16_t>(addr, 0x0600 + (i * 16) + 2, wkc);
        if (len > 0) {
            uint32_t log = enumerator.read_register_fprd<uint32_t>(addr, 0x0600 + (i * 16), wkc);
            std::cout << std::left << std::setw(6) << i << "0x" << std::hex << std::setw(10) << log << std::dec << std::setw(10) << len 
                      << std::setw(10) << "-" << std::setw(10) << "-" << "0x" << std::hex << std::setw(10) << phy << std::dec << "PDO" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    optparse::OptionParser parser = optparse::OptionParser().description("Quasar EtherCAT Diagnostic Utility");

    parser.add_option("-i", "--interface").dest("interface").set_default("eth0").help("Network interface (default: eth0)");
    parser.add_option("-s", "--slave").dest("slave").type("int").set_default(-1).help("Target slave index (default: all)");

    optparse::Values options = parser.parse_args(argc, argv);
    std::string iface = options["interface"];
    int target_slave = -1;
    if (options.is_set("slave")) {
        target_slave = options.get("slave");
    }

    try {
        RawSocket socket(iface);
        Enumerator enumerator(socket);
        Result<size_t> result = enumerator.enumerate();
        if (result == false) {
            std::cerr << "Enumeration failed." << std::endl;
            return 1;
        }
        int count = static_cast<int>(result.value());

        for (int i = 0; i < count; ++i) {
            if (target_slave != -1 && i != target_slave) continue;

            const SlaveInfo& slave = enumerator.slaves()[static_cast<size_t>(i)];
            int wkc = 0;
            uint16_t al_status = enumerator.read_register_fprd<uint16_t>(slave.configured_address, regs::AL_STATUS, wkc);
            uint16_t al_code = enumerator.read_register_fprd<uint16_t>(slave.configured_address, 0x0134, wkc);

            std::cout << "\n=== Slave " << i << " [" << slave.name << "] ===" << std::endl;
            std::cout << "State:      " << std::hex << "0x" << (al_status & 0x0F) << " (" << al_status_code_to_string(al_status & 0x0F) << ")" << std::dec << std::endl;
            if (al_code != 0) {
                std::cout << "AL Error:   " << std::hex << "0x" << al_code << " (" << al_status_code_to_string(al_code) << ")" << std::dec << std::endl;
            }

            // Error Counters
            uint8_t rx_err0 = enumerator.read_register_fprd<uint8_t>(slave.configured_address, 0x0300, wkc);
            uint8_t rx_err1 = enumerator.read_register_fprd<uint8_t>(slave.configured_address, 0x0302, wkc);
            std::cout << "RX Errors:  Port0: " << (int)rx_err0 << ", Port1: " << (int)rx_err1 << std::endl;

            if (target_slave != -1) {
                print_fmmu(enumerator, slave.configured_address);
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
