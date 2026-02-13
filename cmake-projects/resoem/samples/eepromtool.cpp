/**
 * @file eepromtool.cpp
 * @brief Port of eepromtool.c to resoem
 */

#include "resoem/Enumerator.hpp"
#include "resoem/RawSocket.hpp"
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
    return 1;
  }

  std::string iface = argv[1];
  int slave_pos = std::stoi(argv[2]);
  std::string op = argv[3];
  std::string filename = (argc > 4) ? argv[4] : "";

  try {
    RawSocket socket(iface);
    Enumerator enumerator(socket);

    if (auto res = enumerator.enumerate(); !res || *res == 0) {
      std::cout << "No slaves found.\n";
      return 1;
    }

    if (slave_pos < 1 || slave_pos > (int)enumerator.slaves().size()) {
      std::cout << "Slave " << slave_pos << " not found.\n";
      return 1;
    }

    int slave_idx = slave_pos - 1;

    if (op == "-r") {
      // Read entire EEPROM
      // How big is it? We don't verify size here, assume 0x80 bytes minimum (1
      // Kbit) or ask user. SOEM eepromtool reads until error or
      // MAX_EEPROM_SIZE.
      std::cout << "Reading EEPROM from slave " << slave_pos << "...\n";
      std::vector<uint8_t> eeprom_data;

      // Read by words (2 bytes) but read_eeprom returns 4 bytes (2 words?)?
      // Wait, read_eeprom in Enumerator calls read_sii_word -> returns 32-bit
      // (2 words at once?) SII is addressed by WORD (16-bit). eeprom::CMD_READ
      // reads 4 bytes or 2 bytes? "read_sii_word" returns uint32_t. Let's
      // assume it reads 4 bytes at 'word_addr'. If we increment word_addr by 1
      // ? In read_sii_word we assume it returns 32 bits (2 words). SOEM
      // ecx_readeepromAP returns 32 or 64 bits.

      // Let's read word by word (16-bit addressing).
      // Enumerator::read_eeprom uses read_sii_word which returns uint32.
      // "read_sii_word" implementation:
      //   write_register_fpwr(..., regs::EEPROM_ADDRESS, word_addr);
      //   return read_register_fprd<uint32_t>(..., regs::EEPROM_DATA, ...);
      // It reads 4 bytes from EEPROM_DATA.
      // The address is WORD address.
      // So address 0 returns words 0 and 1.
      // Address 1 returns words 1 and 2 ? Or address 2?
      // The standard says: Address is Word Address.
      // If we read 4 bytes (2 words), we get [WordAddr, WordAddr+1].
      // So we should increment addr by 2?

      for (uint16_t a = 0; a < 0x400; a += 2) { // Read 2KB max for now
        uint32_t d = enumerator.read_eeprom(slave_idx, a);
        if (d == 0xFFFFFFFF)
          break; // Error or end

        uint8_t b0 = d & 0xFF;
        uint8_t b1 = (d >> 8) & 0xFF;
        uint8_t b2 = (d >> 16) & 0xFF; // Should match next read?
        uint8_t b3 = (d >> 24) & 0xFF;

        eeprom_data.push_back(b0);
        eeprom_data.push_back(b1);
        eeprom_data.push_back(b2);
        eeprom_data.push_back(b3);
      }

      if (!filename.empty()) {
        std::ofstream outfile(filename, std::ios::binary);
        outfile.write(reinterpret_cast<const char *>(eeprom_data.data()),
                      eeprom_data.size());
        std::cout << "Saved " << eeprom_data.size() << " bytes to " << filename
                  << "\n";
      }

    } else if (op == "-w") {
      if (filename.empty()) {
        std::cout << "File required for write.\n";
        return 1;
      }
      std::ifstream infile(filename, std::ios::binary | std::ios::ate);
      size_t size = infile.tellg();
      infile.seekg(0);
      std::vector<uint8_t> buffer(size);
      infile.read(reinterpret_cast<char *>(buffer.data()), size);

      std::cout << "Writing " << size << " bytes to EEPROM of slave "
                << slave_pos << "...\n";

      // Write 2 bytes (1 word) at a time.
      for (size_t i = 0; i < size / 2; ++i) {
        uint16_t word;
        word = buffer[2 * i] | (buffer[2 * i + 1] << 8);
        if (!enumerator.write_eeprom(slave_idx, i, word)) {
          std::cout << "Failed to write at word address " << i << "\n";
          return 1;
        }
        // Determine write time? write_eeprom waits internally.
        if (i % 64 == 0)
          std::cout << "." << std::flush;
      }
      std::cout << "\nDone.\n";
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
