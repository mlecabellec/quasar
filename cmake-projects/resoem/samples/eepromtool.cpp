/**
 * @file eepromtool.cpp
 * @brief EtherCAT SII (EEPROM) read/write utility (Resoem).
 * @details This tool allows reading from and writing to the Slave Information Interface (SII) 
 * of an EtherCAT slave. SII contains the device identity, capabilities, and configurations.
 * 
 * NOTE: SII is addressed by 16-bit WORDS. A 1024-bit EEPROM has 64 words.
 */

#include "resoem/Enumerator.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/Diagnostics.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace resoem;

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cout << "Usage: eepromtool IFNAME SLAVE OP [FILE]\n";
    std::cout << "  OP: -r (read), -w (write), -i (info)\n";
    std::cout << "Example: eepromtool eth0 1 -r backup.bin\n";
    return 1;
  }

  std::string iface = argv[1];
  int slave_pos = std::stoi(argv[2]);
  std::string op = argv[3];
  std::string filename = (argc > 4) ? argv[4] : "";

  try {
    std::cout << "Connecting to " << iface << "..." << std::endl;
    RawSocket socket(iface);
    Enumerator enumerator(socket);

    std::cout << "Scanning for slaves..." << std::endl;
    if (auto res = enumerator.enumerate(); !res || *res == 0) {
      std::cout << "[ERROR] No slaves found on the bus.\n";
      return 1;
    }

    if (slave_pos < 1 || slave_pos > (int)enumerator.slaves().size()) {
      std::cout << "[ERROR] Slave " << slave_pos << " not found (Bus has " 
                << enumerator.slaves().size() << " slaves).\n";
      return 1;
    }

    int slave_idx = slave_pos - 1;
    const auto& slave = enumerator.slaves()[slave_idx];
    std::cout << "[INFO] Target Slave: " << slave.name << " (Addr: 0x" 
              << std::hex << slave.configured_address << std::dec << ")\n";

    if (op == "-r") {
      std::cout << "[OPERATION] Reading EEPROM content...\n";
      std::vector<uint8_t> eeprom_data;

      // SII is Word-addressed (16-bit). We read in 32-bit or 64-bit chunks.
      // Resoem's read_eeprom returns 32 bits (2 words) starting at word_addr.
      // We read up to 2KB (typical max for small devices) or until error.
      for (uint16_t a = 0; a < 0x400; a += 2) { 
        uint32_t d = enumerator.read_eeprom(slave_idx, a);
        if (d == 0xFFFFFFFF) {
          std::cout << "\n[INFO] End of EEPROM or read error at word 0x" << std::hex << a << std::dec << "\n";
          break; 
        }

        eeprom_data.push_back(static_cast<uint8_t>(d & 0xFF));
        eeprom_data.push_back(static_cast<uint8_t>((d >> 8) & 0xFF));
        eeprom_data.push_back(static_cast<uint8_t>((d >> 16) & 0xFF));
        eeprom_data.push_back(static_cast<uint8_t>((d >> 24) & 0xFF));
        
        if (a % 32 == 0) std::cout << "." << std::flush;
      }

      if (!filename.empty()) {
        std::ofstream outfile(filename, std::ios::binary);
        outfile.write(reinterpret_cast<const char *>(eeprom_data.data()),
                      eeprom_data.size());
        std::cout << "\n[SUCCESS] Saved " << eeprom_data.size() << " bytes to " << filename << "\n";
      } else {
          std::cout << "\n[INFO] Read " << eeprom_data.size() << " bytes (Hex dump omitted, provide filename to save).\n";
      }

    } else if (op == "-w") {
      if (filename.empty()) {
        std::cout << "[ERROR] File required for write operation.\n";
        return 1;
      }
      std::ifstream infile(filename, std::ios::binary | std::ios::ate);
      if (!infile) {
          std::cout << "[ERROR] Could not open file: " << filename << "\n";
          return 1;
      }
      size_t size = infile.tellg();
      infile.seekg(0);
      std::vector<uint8_t> buffer(size);
      infile.read(reinterpret_cast<char *>(buffer.data()), size);

      std::cout << "[OPERATION] Writing " << size << " bytes to EEPROM...\n";
      std::cout << "[WARNING] Writing to SII can brick your device if data is incorrect. Proceeding...\n";

      // Write 2 bytes (1 word) at a time.
      for (size_t i = 0; i < size / 2; ++i) {
        uint16_t word = buffer[2 * i] | (buffer[2 * i + 1] << 8);
        if (!enumerator.write_eeprom(slave_idx, static_cast<uint16_t>(i), word)) {
          std::cerr << "\n[ERROR] Failed to write at word address " << i << "\n";
          int wkc;
          uint16_t stat = enumerator.read_register_fprd<uint16_t>(slave.configured_address, regs::EEPROM_CONTROL, wkc);
          if (wkc > 0) {
              std::cerr << "        EEPROM Status: 0x" << std::hex << stat << std::dec << "\n";
              if (stat & eeprom::ERROR_MASK) std::cerr << "        Error bits set in ESC EEPROM Control.\n";
          }
          return 1;
        }
        if (i % 8 == 0) std::cout << "." << std::flush;
      }
      std::cout << "\n[SUCCESS] Write complete.\n";
      
      std::cout << "[INFO] Requesting EEPROM Reload..." << std::endl;
      // Many slaves require a reload or power cycle to apply SII changes.
      // We can try to issue a Reload command via register 0x502
      // enumerator.write_register_fpwr(slave.configured_address, regs::EEPROM_CONTROL, eeprom::CMD_RELOAD);
    } else if (op == "-i") {
        std::cout << "[INFO] SII Basic Information:\n";
        std::cout << "  - Vendor:   0x" << std::hex << slave.vendor_id << std::dec << "\n";
        std::cout << "  - Product:  0x" << std::hex << slave.product_code << std::dec << "\n";
        std::cout << "  - Revision: 0x" << std::hex << slave.revision_number << std::dec << "\n";
    }
  } catch (const std::exception &e) {
    std::cerr << "[FATAL] " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
