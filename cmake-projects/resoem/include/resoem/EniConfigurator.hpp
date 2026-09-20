/**
 * @file EniConfigurator.hpp
 * @brief High-performance network configuration engine executing ETG.2100 ENI rules.
 * @details Contributes to [TSK-20260920-001.2] Network Configuration Engine and [FE-0040.5].
 */

#pragma once

#include "resoem/EniConstants.hpp"
#include "resoem/EniDocument.hpp"
#include "resoem/IRawSocket.hpp"
#include "resoem/common.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>

namespace resoem::eni {

/**
 * @struct TransitionReport
 * @brief Detailed report of an ESM state transition execution.
 */
struct TransitionReport {
  bool success{true};                        ///< Overall success status
  std::size_t failed_slave_idx{0};           ///< Index of first failed slave if any
  std::string failed_command_comment;        ///< Comment of failed InitCmd
  std::string error_message;                 ///< Explanatory error description
  std::uint16_t al_status_code{0};           ///< ESC AL Status code on failure
};

/**
 * @class EniConfigurator
 * @brief Configures physical or virtual EtherCAT networks according to an ETG.2100 ENI document.
 * @details Sequentially executes Master and Slave InitCmds per ESM transition (IP, PS, SO, OP),
 * evaluates <Validate> bitmask conditions with configurable retries, programs FMMUs and
 * SyncManagers, configures Distributed Clocks, and dispatches multi-datagram cyclic frames.
 */
class EniConfigurator {
public:
  /**
   * @brief Construct an EniConfigurator.
   * @param socket Transport socket (physical RawSocket or virtual StubRawSocket).
   * @param document Parsed ENI AST structure.
   */
  explicit EniConfigurator(IRawSocket &socket, const EniDocument &document);

  /**
   * @brief Virtual destructor.
   */
  virtual ~EniConfigurator() noexcept = default;

  // Prevent copying
  EniConfigurator(const EniConfigurator &) = delete;
  EniConfigurator &operator=(const EniConfigurator &) = delete;

  /**
   * @brief Configure the entire network from INIT to OPERATIONAL (OP).
   * @details Executes all Master/Slave InitCmds for IP, PS, SO, and transitions all
   * reachable slaves to OP state.
   * @return Result<> indicating success or error.
   */
  [[nodiscard]] Result<void> configure_all();

  /**
   * @brief Execute all InitCmds and state requests for a specific ESM transition.
   * @param transition Target transition (e.g. EsmTransition::IP, EsmTransition::PS).
   * @return Result<> indicating success or error.
   */
  [[nodiscard]] Result<void> run_transition(EsmTransition transition);

  /**
   * @brief Execute Master InitCmds for the specified transition.
   * @param transition Target transition.
   * @return Result<> indicating success or error.
   */
  [[nodiscard]] Result<void> configure_master_init_cmds(EsmTransition transition);

  /**
   * @brief Execute Slave InitCmds for a specific slave and transition.
   * @param slave_idx 0-based slave index in the ENI document.
   * @param transition Target transition.
   * @return Result<> indicating success or error.
   */
  [[nodiscard]] Result<void> configure_slave_init_cmds(std::size_t slave_idx, EsmTransition transition);

  /**
   * @brief Program FMMU and SyncManager registers for the specified slave.
   * @param slave_idx 0-based slave index.
   * @return Result<> indicating success or error.
   */
  [[nodiscard]] Result<void> configure_fmmu_and_sm(std::size_t slave_idx);

  /**
   * @brief Program Distributed Clocks (DC) registers for the specified slave.
   * @param slave_idx 0-based slave index.
   * @return Result<> indicating success or error.
   */
  [[nodiscard]] Result<void> configure_dc(std::size_t slave_idx);

  /**
   * @brief Request and wait for a specific slave to transition to target ESM state.
   * @param slave_idx 0-based slave index.
   * @param target_state Target state bitmask (e.g. states::PRE_OP, states::OP).
   * @param timeout Timeout duration.
   * @return True if target state reached.
   */
  [[nodiscard]] bool request_slave_state(std::size_t slave_idx, std::uint16_t target_state,
                                         std::chrono::milliseconds timeout = std::chrono::milliseconds(200));

  /**
   * @brief Transmit and receive a cyclic frame defined in <Cyclic><Frame>.
   * @param frame_idx 0-based frame index.
   * @param process_image_data Buffer for process image input/output data.
   * @return Working Counter (WKC) or error.
   */
  [[nodiscard]] Result<std::uint16_t> exchange_cyclic_frame(std::size_t frame_idx,
                                                            std::span<byte> process_image_data);

  /**
   * @brief Get the number of configured slaves in the ENI document.
   * @return Slave count.
   */
  [[nodiscard]] std::size_t slave_count() const noexcept;

  /**
   * @brief Get the last transition report.
   * @return Constant reference to TransitionReport.
   */
  [[nodiscard]] const TransitionReport &last_report() const noexcept;

  /**
   * @brief Set whether mandatory slave failures abort configuration.
   * @param abort If true, aborts immediately on mandatory failure.
   */
  void set_abort_on_mandatory_failure(bool abort) noexcept;

  /**
   * @brief Set default maximum retry count for InitCmd execution.
   * @param retries Maximum retries.
   */
  void set_max_retries(std::uint32_t retries) noexcept;

private:
  mutable std::timed_mutex m_mutex;        ///< Concurrency guard
  IRawSocket &m_socket;                    ///< Transport socket
  const EniDocument &m_document;           ///< Target ENI document
  std::uint8_t m_currentIdx{0};            ///< Cyclic datagram index
  bool m_abortOnMandatory{true};           ///< Failure policy
  std::uint32_t m_defaultRetries{DEFAULT_INIT_CMD_RETRIES}; ///< Default retries
  TransitionReport m_lastReport{};         ///< Diagnostics report

  // Internal execution helpers
  [[nodiscard]] bool execute_single_init_cmd(const EniInitCmd &cmd, std::size_t slave_idx,
                                             bool is_optional);
  [[nodiscard]] bool validate_data(std::span<const byte> received, const ValidateRule &rule) const;
  [[nodiscard]] int send_receive_datagram(std::uint8_t cmd, std::uint16_t addr,
                                         std::uint16_t offset, std::span<byte> data);
  [[nodiscard]] bool transition_slaves_to_state(std::uint16_t target_state,
                                               std::chrono::milliseconds timeout);
  [[nodiscard]] static bool matches_transition(EsmTransition cmd_transition,
                                               EsmTransition target) noexcept;
};

} // namespace resoem::eni
