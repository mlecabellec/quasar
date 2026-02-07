#pragma once

#include "resoem/RawSocket.hpp"
#include "resoem/Slave.hpp"
#include "resoem/common.hpp"
#include <span>
#include <string>
#include <vector>

namespace resoem {

class Enumerator {
public:
  Enumerator(RawSocket &socket);

  // Run the full enumeration process
  // Returns the number of slaves found
  int enumerate();

  const std::vector<SlaveInfo> &slaves() const { return slaves_; }

private:
  RawSocket &socket_;
  std::vector<SlaveInfo> slaves_;
  uint8_t current_idx_ = 0;

  int broadcast_read_count();
  void reset_to_init();
  void assign_addresses(int count);
  void read_sii_data(int count);
  void read_sii_categories(int slave_idx);
  void map_topology(int count);

  // Helpers to send datagrams
  // Returns WKC
  int send_receive(uint8_t cmd, uint16_t addr, uint16_t offset,
                   std::span<byte> data);

  // Type-safe wrappers
  template <typename T> T read_register_broadcast(uint16_t reg, int &wkc);

  template <typename T>
  int write_register_broadcast(uint16_t reg, const T &value);

  template <typename T>
  T read_register_aprd(uint16_t auto_inc_addr, uint16_t reg, int &wkc);

  template <typename T>
  int write_register_apwr(uint16_t auto_inc_addr, uint16_t reg, const T &value);

  template <typename T>
  T read_register_fprd(uint16_t configured_addr, uint16_t reg, int &wkc);

  template <typename T>
  int write_register_fpwr(uint16_t configured_addr, uint16_t reg,
                          const T &value);

  // EEPROM helpers
  uint32_t read_sii_word(uint16_t slave_cfg_addr, uint16_t word_addr);
  uint16_t find_sii_category(uint16_t slave_cfg_addr, uint16_t cat_type);
  std::string read_sii_string(uint16_t slave_cfg_addr, uint8_t string_idx);
};

} // namespace resoem
