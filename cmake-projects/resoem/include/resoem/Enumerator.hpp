/**
 * @file Enumerator.hpp
 * @brief Class for discovering and configuring EtherCAT slaves on the network.
 */

#pragma once

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
  /**
   * @brief Construct a new Enumerator.
   * @param socket The raw socket to use for communication.
   */
  explicit Enumerator(RawSocket &socket);

  /**
   * @brief Run the full enumeration process.
   * @details [FE-0040.3.1] Automatically detect and count slaves.
   * [FE-0040.3.2] Assign configured station addresses to each slave.
   * [FE-0040.3.3] Parse SII to identify Vendor, Product, etc.
   * [FE-0040.3.4] Map network topology.
   *
   * This includes resetting the network, counting slaves, assigning addresses,
   * and reading device information from the SII (EEPROM).
   *
   * @return Result<size_t> Number of slaves found or an error code.
   */
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

  /**
   * @brief Configure FMMUs (Fieldbus Memory Management Units) for process data.
   * @details [FE-0040.5.2] Automatically calculate and program FMMU entries.
   *
   * Maps slave physical memory (PDOs) to the logical process image.
   *
   * @param image The process image to configure.
   * @return Result<uint32_t> Total logical size in bytes.
   */
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

  /**
   * @brief Read a 32-bit word from the slave's SII (EEPROM).
   *
   * @param slave_idx Index of the slave.
   * @param word_addr Word address in the EEPROM.
   * @return uint32_t The value read or 0xFFFFFFFF on failure.
   */
  uint32_t read_eeprom(uint16_t slave_idx, uint16_t word_addr);

  /**
   * @brief Write a 16-bit word to the slave's SII (EEPROM).
   *
   * @param slave_idx Index of the slave.
   * @param word_addr Word address in the EEPROM.
   * @param data The 16-bit data to write.
   * @return int 1 on success, 0 on failure.
   */
  int write_eeprom(uint16_t slave_idx, uint16_t word_addr, uint16_t data);

  /**
   * @brief Load configuration from an ENI structure.
   *
   * Applies CoE initialization commands from the ENI to the discovered
   * slaves.
   *
   * @param eni Pointer to the ENI structure (generated by eniconv).
   */
  void load_eni(const ec_enit *eni);

  /**
   * @brief Measure propagation delays between slaves for Distributed Clocks
   * (DC).
   * @details [FE-0040.6.1] Implement propagation delay measurement with ns resolution.
   */
  void measure_propagation_delays();

  /**
   * @brief Synchronize all slave clocks to the Reference Clock (usually the
   * first DC-capable slave).
   * @details [FE-0040.6.3] Provide cyclic drift compensation using ARMW command.
   */
  void sync_clocks();

  /**
   * @brief Configure SYNC0/SYNC1 signals for a specific slave.
   * @details [FE-0040.6.4] Configure SYNC0/SYNC1 signals with cycle times and offsets.
   *
   * @param slave The slave info structure.
   * @param cycle_time Sync cycle time in nanoseconds.
   * @param shift_time Sync shift time in nanoseconds.
   */
  void configure_dc(const SlaveInfo &slave, uint32_t cycle_time, int32_t shift_time);

  /**
   * @brief Read the ESC error counters (CRC errors, link loss, etc.) for all
   * slaves.
   *
   * This updates the `error_counters` field in each slave's info structure.
   * @return Result<void> Success or error.
   */
  Result<> read_error_counters();

  /**
   * @brief Check status of all slaves and update 'online' and
   * 'current_state' fields.
   */
  void check_slaves_status();

  /**
   * @brief Scan for topology changes (gaining or losing slaves).
   * @details Detects if slaves have been added or removed without resetting 
   * the entire bus.
   * @return Result<bool> True if topology changed.
   */
  Result<bool> check_topology();

  /**
   * @brief Reconfigure a specific slave.
   * @param slave_idx Index of the slave.
   * @return Result<> Success or error.
   */
  Result<> reconfigure_slave(uint16_t slave_idx);

  /**
   * @brief Reconfigure all slaves on the bus.
   * @return Result<> Success or error.
   */
  Result<> reconfigure_all();

  /**
   * @brief Attempt to recover a slave that has gone offline or errored.
   * @details [FE-0040.7.1] Automated slave recovery.
   *
   * @param slave_idx Index of the slave.
   * @return true if recovery was successful.
   */
  bool recover_slave(int slave_idx);

  /**
   * @brief Get the list of discovered slaves.
   * @return Constant reference to the slave list.
   */
  const std::vector<SlaveInfo> &slaves() const { return slaves_; }

  /**
   * @brief Enable or disable verbose logging.
   * @param level Verbosity level (0=off, 1=normal, 2=high).
   */
  void set_verbose(int level) { verbose_level_ = level; }

  /**
   * @brief Check if verbose logging is enabled.
   * @return Current verbosity level.
   */
  int verbose() const { return verbose_level_; }

  // Low-level register access (Public for diagnostics)
  template <typename T>
  T read_register_fprd(uint16_t configured_addr, uint16_t reg, int &wkc);
  template <typename T>
  int write_register_fpwr(uint16_t configured_addr, uint16_t reg,
                          const T &value);
  template <typename T>
  T read_register_aprd(uint16_t auto_inc_addr, uint16_t reg, int &wkc);
  template <typename T>
  int write_register_apwr(uint16_t auto_inc_addr, uint16_t reg, const T &value);
  template <typename T> T read_register_broadcast(uint16_t reg, int &wkc);
  template <typename T>
  int write_register_broadcast(uint16_t reg, const T &value);

private:
  RawSocket &socket_;             ///< Raw socket reference
  std::vector<SlaveInfo> slaves_; ///< List of discovered slaves
  uint8_t current_idx_ = 0;       ///< Cyclic index for datagrams
  int verbose_level_ = 0;         ///< Verbose logging level

public:
  void reset_to_init();

private:
  // Internal helper methods
  int broadcast_read_count();
  void assign_addresses(int count);
  void read_sii_data(int count);
  void read_sii_categories(int slave_idx);
  void read_sii_pdos(int slave_idx);
  void map_topology(int count);
    // Low-level SII helpers (private)
    uint32_t read_sii_word(uint16_t slave_cfg_addr, uint16_t word_addr);
    SIICategory find_sii_category(uint16_t slave_cfg_addr, uint16_t cat_type);
    std::string read_sii_string(uint16_t slave_cfg_addr, uint8_t string_idx);
    void read_port_status();
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

  /**
   * @brief Perform one polling tick for a single slave in the state FSM.
   * @details Called each iteration of the request_state_all() poll loop.
   * If the slave has not yet been sent the AL_CONTROL request, writes it
   * via FPWR. Then reads AL_STATUS and, if the error bit is set, reads
   * AL_STATUS_CODE and acknowledges the error. Marks fsm.done when the
   * slave reaches the target state.
   *
   * @param slave_idx  Index in slaves_ list.
   * @param fsm        Per-slave FSM state (modified in place).
   */
  void advance_slave_state(size_t slave_idx, SlaveStateFSM &fsm);

  int send_receive(uint8_t cmd, uint16_t addr, uint16_t offset,
                   std::span<byte> data);
};

} // namespace resoem
