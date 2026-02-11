#pragma once

#include "resoem/RawSocket.hpp"
#include "resoem/Slave.hpp"
#include "resoem/common.hpp"
#include <chrono>
#include <span>
#include <string>
#include <vector>

namespace resoem {

class ProcessImage;

class Enumerator {
public:
  Enumerator(RawSocket &socket);

  /**
   * Run the full enumeration process.
   * Returns the number of slaves found or an error.
   */
  Result<size_t> enumerate();

  /**
   * Request a state transition for a slave.
   * Returns the actual state reached.
   */
  Result<uint16_t> request_state(
      uint16_t slave_idx, uint16_t state,
      std::chrono::microseconds timeout = std::chrono::seconds(3));

  /**
   * Request state transition for ALL slaves.
   */
  Result<> request_state_all(
      uint16_t state,
      std::chrono::microseconds timeout = std::chrono::seconds(3));

  /**
   * Configure FMMUs and return total logical size.
   */
  Result<uint32_t> configure_fmmu(ProcessImage &image);

  /**
   * Exchange process data logical image.
   * Returns the Working Counter (WKC).
   */
  Result<uint16_t> exchange_process_data(
      ProcessImage &image,
      std::chrono::microseconds timeout = std::chrono::milliseconds(2));

  /**
   * Measure propagation delays for Distributed Clocks (DC).
   */
  void measure_propagation_delays();

  /**
   * Synchronize slave clocks to the Reference Clock.
   */
  void sync_clocks();

  /**
   * Configure SYNC0/SYNC1 for a slave.
   */
  void configure_dc(SlaveInfo &slave, uint32_t cycle_time, int32_t shift_time);

  /**
   * Check status of all slaves and update 'online' and 'current_state' fields.
   */
  void check_slaves_status();

  /**
   * Attempt to recover a slave that has gone offline or errored.
   */
  bool recover_slave(int slave_idx);

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
  void read_sii_pdos(int slave_idx);
  void map_topology(int count);

  int send_receive(uint8_t cmd, uint16_t addr, uint16_t offset, std::span<byte> data);

  template <typename T> T read_register_broadcast(uint16_t reg, int &wkc);
  template <typename T> int write_register_broadcast(uint16_t reg, const T &value);
  template <typename T> T read_register_aprd(uint16_t auto_inc_addr, uint16_t reg, int &wkc);
  template <typename T> int write_register_apwr(uint16_t auto_inc_addr, uint16_t reg, const T &value);
  template <typename T> T read_register_fprd(uint16_t configured_addr, uint16_t reg, int &wkc);
  template <typename T> int write_register_fpwr(uint16_t configured_addr, uint16_t reg, const T &value);

  uint32_t read_sii_word(uint16_t slave_cfg_addr, uint16_t word_addr);
  uint16_t find_sii_category(uint16_t slave_cfg_addr, uint16_t cat_type);
  std::string read_sii_string(uint16_t slave_cfg_addr, uint8_t string_idx);
};

} // namespace resoem
