/**
 * @file Enumerator.hpp
 * @brief Class for discovering and configuring EtherCAT slaves on the network.
 */

#pragma once

#include "resoem/IRawSocket.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/Slave.hpp"
#include "resoem/common.hpp"
#include "resoem/soem_eni.hpp"
#include <chrono>
#include <span>
#include <string>
#include <vector>

namespace resoem {

class ProcessImage;

/**
 * @brief Manages the discovery, initialization, and monitoring of EtherCAT
 * slaves.
 * @details Contributes to [FE-0040.3] Network Discovery & Enumeration,
 * [FE-0040.5] Process Data Configuration, [FE-0040.6] DC Synchronization,
 * and [FE-0040.7] Resilience & Diagnostics.
 *
 * The Enumerator is responsible for scanning the network, assigning station
 * addresses, reading slave information (SII), and handling state transitions.
 */
class Enumerator {
public:
  /// @brief Construct a new Enumerator bound to socket.
  explicit Enumerator(IRawSocket &socket);

  /// @brief Run full network enumeration ([FE-0040.3.1]..[FE-0040.3.4]).
  Result<size_t> enumerate();

  /**
   * @brief Request a state transition for a specific slave.
   *
   * @param slave_idx Index of the slave in the internal list.
   * @param state Target AL state (e.g., states::PRE_OP).
   * @param timeout Maximum time to wait for the transition.
   * @return Result<uint16_t> The actual state reached or an error.
   */
  Result<uint16_t>
  request_state(uint16_t slave_idx, uint16_t state,
                std::chrono::microseconds timeout = std::chrono::seconds(3));

  /**
   * @brief Per-slave outcome from a state transition attempt.
   * @details Populated by request_state_all(); one entry per slave.
   */
  struct SlaveTransitionResult {
    size_t   slave_idx;      ///< Index in the slaves() list
    bool     success;        ///< True if slave reached the target state
    uint16_t final_state;   ///< AL state bits at end of transition
    uint16_t error_code;    ///< Last AL status code (0 = no error)
  };

  /**
   * @brief Request a state transition for ALL slaves individually (no broadcast).
   * @details Each slave receives its own FPWR AL_CONTROL write and is polled
   * independently. Slaves that reach the target before the timeout do not block
   * others. Slaves with ERROR_BIT set are acknowledged per-slave and
   * retried up to the timeout deadline.
   *
   * On timeout the function still returns an ok Result if ALL slaves succeeded;
   * returns ECError::Timeout only if at least one slave never reached the target.
   *
   * Per-slave details are always printed to stderr on failure regardless of
   * verbose_level.
   *
   * @param state   Target AL state (e.g. states::PRE_OP, states::SAFE_OP).
   * @param timeout Maximum wall-clock time for the entire operation.
   * @return Result<> ok, or ECError::Timeout if any slave failed.
   */
  Result<> request_state_all(uint16_t state, std::chrono::microseconds timeout =
                                                 std::chrono::seconds(3));

  /// @brief Configure FMMUs for process data ([FE-0040.5.2]).
  Result<uint32_t> configure_fmmu(ProcessImage &image);

  /**
   * @brief Exchange process data using a Logical ReadWrite (LRW) command.
   * @details [FE-0040.5.4] Support cyclic data exchange using the LRW command.
   *
   * @param image The process image containing outputs to send and buffer for
   * inputs.
   * @param timeout Receive timeout.
   * @return Result<uint16_t> Working Counter (WKC) from the datagram.
   */
  Result<uint16_t> exchange_process_data(
      ProcessImage &image,
      std::chrono::microseconds timeout = std::chrono::milliseconds(2));

  /**
   * @brief Represents a category in the Slave Information Interface (SII).
   */
  struct SIICategory {
    uint16_t offset;         ///< Word offset of the category data
    uint16_t size_in_words;  ///< Size of the category data in words
  };

  /// @brief Read a 32-bit word from the slave's SII (EEPROM).
  uint32_t read_eeprom(uint16_t slave_idx, uint16_t word_addr);

  /// @brief Write a 16-bit word to the slave's SII (EEPROM).
  int write_eeprom(uint16_t slave_idx, uint16_t word_addr, uint16_t data);

  /// @brief Load CoE initialization commands from an ENI structure.
  void load_eni(const ec_enit *eni);

  /// @brief Measure propagation delays between slaves for Distributed Clocks ([FE-0040.6.1]).
  void measure_propagation_delays();

  /// @brief Synchronize all slave clocks to the Reference Clock ([FE-0040.6.3]).
  void sync_clocks();

  /// @brief Configure SYNC0/SYNC1 signals for a specific slave ([FE-0040.6.4]).
  void configure_dc(const SlaveInfo &slave, uint32_t cycle_time, int32_t shift_time);

  /// @brief Read ESC error counters for all slaves.
  Result<> read_error_counters();

  /// @brief Check status of all slaves and update online and current_state fields.
  void check_slaves_status();

  /// @brief Scan for topology changes without full bus reset.
  Result<bool> check_topology();

  /// @brief Reconfigure a specific slave by index.
  Result<> reconfigure_slave(uint16_t slave_idx);

  /// @brief Reconfigure all slaves on the bus.
  Result<> reconfigure_all();

  /// @brief Attempt to recover a slave that has gone offline or errored ([FE-0040.7.1]).
  bool recover_slave(int slave_idx);

  /**
   * @brief Get the list of discovered slaves.
   * @return Constant reference to the slave list.
   */
  const std::vector<SlaveInfo> &slaves() const { return slaves_; }

  /// @brief Enable or disable verbose logging.
  void set_verbose(int level) { verbose_level_ = level; }

  /// @brief Check current verbosity level.
  int verbose() const { return verbose_level_; }

  /// @brief Read ESC register via FPRD.
  template <typename T>
  T read_register_fprd(uint16_t configured_addr, uint16_t reg, int &wkc);
  /// @brief Write ESC register via FPWR.
  template <typename T>
  int write_register_fpwr(uint16_t configured_addr, uint16_t reg,
                          const T &value);
  /// @brief Read ESC register via APRD.
  template <typename T>
  T read_register_aprd(uint16_t auto_inc_addr, uint16_t reg, int &wkc);
  /// @brief Write ESC register via APWR.
  template <typename T>
  int write_register_apwr(uint16_t auto_inc_addr, uint16_t reg, const T &value);
  /// @brief Broadcast read ESC register via BRD.
  template <typename T> T read_register_broadcast(uint16_t reg, int &wkc);
  /// @brief Broadcast write ESC register via BWR.
  template <typename T>
  int write_register_broadcast(uint16_t reg, const T &value);

private:
  IRawSocket &socket_;            ///< Raw socket reference (physical or virtual)
  std::vector<SlaveInfo> slaves_; ///< List of discovered slaves
  uint8_t current_idx_ = 0;       ///< Cyclic index for datagrams
  int verbose_level_ = 0;         ///< Verbose logging level

public:
  void reset_to_init();

private:
  /// @brief Broadcast read slave count.
  int broadcast_read_count();
  /// @brief Assign configured station addresses to discovered slaves.
  void assign_addresses(int count);
  /// @brief Read basic SII identity data for all slaves.
  void read_sii_data(int count);
  /// @brief Read SII categories for a slave.
  void read_sii_categories(int slave_idx);
  /// @brief Parse SII PDO categories for a slave.
  void read_sii_pdos(int slave_idx);
  /// @brief Map physical port topology across slaves.
  void map_topology(int count);
  /// @brief Read a 32-bit word from slave SII EEPROM.
  uint32_t read_sii_word(uint16_t slave_cfg_addr, uint16_t word_addr);
  /// @brief Locate SII category header by type.
  SIICategory find_sii_category(uint16_t slave_cfg_addr, uint16_t cat_type);
  /// @brief Read indexed string from SII strings category.
  std::string read_sii_string(uint16_t slave_cfg_addr, uint8_t string_idx);
  /// @brief Read physical link status of slave ports.
  void read_port_status();
  /// @brief Configure mailbox SyncManagers on a slave.
  void configure_mailbox(uint16_t slave_idx);

  /**
   * @brief Internal per-slave FSM state used by request_state_all().
   * @details Tracks the transition progress for one slave through one
   * call to request_state_all(). All fields are initialised before the
   * polling loop starts and updated on every tick.
   */
  struct SlaveStateFSM {
    uint16_t target_state;    ///< Masked target AL state bits
    bool     done;            ///< True once the slave reached target
    bool     failed;          ///< True if the slave reported a hard error
    uint16_t last_al_status;  ///< Last raw AL_STATUS read
    uint16_t last_error_code; ///< Last AL_STATUS_CODE (0 = none)
    bool     request_written; ///< Whether AL_CONTROL has been written yet
  };

  /// @brief Perform one polling tick for a slave in request_state_all().
  void advance_slave_state(size_t slave_idx, SlaveStateFSM &fsm);

  int send_receive(uint8_t cmd, uint16_t addr, uint16_t offset,
                   std::span<byte> data);
};

} // namespace resoem
