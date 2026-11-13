/**
 * @file HotConnectManager.hpp
 * @brief Dynamic EtherCAT Hot Connect group discovery, ESM stepping, and WKC management.
 * @details Contributes to [TSK-20260920-001.8], [TSK-20260920-001.9], and [FE-0040.8].
 */

#pragma once

#include "resoem/EniConfigurator.hpp"
#include "resoem/EniDocument.hpp"
#include "resoem/HotConnectConstants.hpp"
#include "resoem/IRawSocket.hpp"
#include "resoem/common.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace resoem::hot_connect {

/**
 * @struct HotConnectGroupConfig
 * @brief Configuration and runtime tracking for a single Hot Connect slave group.
 */
struct HotConnectGroupConfig {
  std::string name;                                          ///< Group identifier
  std::vector<std::size_t> slave_indices;                   ///< Indices into EniDocument::slaves()
  std::uint16_t station_alias{0};                           ///< Expected station alias (reg 0x0012)
  std::uint16_t expected_wkc{0};                            ///< WKC contribution when operational
  std::size_t input_process_offset{0};                      ///< Offset in master input process image
  std::size_t input_process_length{0};                      ///< Length in master input process image
  DisconnectedDataPolicy disconnected_policy{DisconnectedDataPolicy::ZERO_FILL}; ///< Disconnect rule
  std::vector<byte> custom_fallback_data;                   ///< Custom fallback bytes
  HotConnectGroupState state{HotConnectGroupState::DISCONNECTED}; ///< Current state
  std::vector<byte> last_known_valid_data;                  ///< Frozen state buffer
  std::size_t missed_cycles{0};                             ///< Missing response counter
};

/**
 * @class HotConnectManager
 * @brief Manages dynamic Hot Connect groups without disturbing running bus slaves.
 * @details Discovers newly plugged groups via station alias identification, performs non-intrusive
 * state machine transitions (INIT -> PRE_OP -> SAFE_OP -> OP) exclusively on group members, dynamically
 * adjusts expected cyclic WKC, and applies configurable fail-safe policies (zero-fill, freeze, custom)
 * to input process images when groups disconnect.
 */
class HotConnectManager {
public:
  /**
   * @brief Construct a HotConnectManager.
   * @param doc Reference to the master ENI document.
   * @param socket Transport socket.
   * @param configurator ENI configurator engine.
   */
  HotConnectManager(const eni::EniDocument &doc, IRawSocket &socket,
                    eni::EniConfigurator &configurator);

  /// @brief Virtual destructor.
  virtual ~HotConnectManager() noexcept = default;

  // Non-copyable via std::timed_mutex member

  /// @brief Register a dynamic Hot Connect group configuration.
  void register_group(const HotConnectGroupConfig &config);

  /**
   * @brief Automatically extract and register all Hot Connect groups from the ENI document.
   */
  void auto_discover_groups_from_eni();

  /**
   * @brief Get the current lifecycle state of a registered group.
   * @param name Group name.
   * @return HotConnectGroupState.
   */
  [[nodiscard]] HotConnectGroupState group_state(const std::string &name) const noexcept;

  /// @brief Configure disconnected data handling policy for a group.
  void set_group_policy(const std::string &name, DisconnectedDataPolicy policy);

  /**
   * @brief Configure disconnected data handling policy with custom fallback bytes.
   * @param name Group name.
   * @param policy Zero-fill, hold last state, or custom fallback.
   * @param fallback_data Custom fallback bytes (if policy is CUSTOM_FALLBACK).
   */
  void set_group_policy(const std::string &name, DisconnectedDataPolicy policy,
                        std::span<const byte> fallback_data);

  /**
   * @brief Calculate current expected cyclic WKC taking active Hot Connect groups into account.
   * @return Dynamic expected WKC.
   */
  [[nodiscard]] std::uint16_t calculate_active_expected_wkc() const noexcept;

  /// @brief Apply disconnected data policies to the master input process image.
  void apply_disconnected_data_policies(std::span<byte> process_image);

  /// @brief Record latest valid inputs for an operational group.
  void record_valid_inputs(const std::string &name, std::span<const byte> inputs);

  /**
   * @brief Execute a periodic scan cycle to discover, identify, and initialize dynamic groups.
   */
  void scan_cycle();

  /// @brief Manually mark a group as disconnected after WKC drop or cable event.
  void mark_group_disconnected(const std::string &name);

  /**
   * @brief Get total number of currently operational Hot Connect groups.
   * @return Count of operational groups.
   */
  [[nodiscard]] std::size_t operational_group_count() const noexcept;

  /**
   * @brief Get total number of registered Hot Connect groups.
   * @return Total registered groups.
   */
  [[nodiscard]] std::size_t total_group_count() const noexcept;

private:
  mutable std::timed_mutex m_mutex;                         ///< Concurrency guard
  const eni::EniDocument &m_doc;                            ///< ENI document reference
  IRawSocket &m_socket;                                     ///< Transport socket
  eni::EniConfigurator &m_configurator;                     ///< Configuration engine
  std::vector<HotConnectGroupConfig> m_groups;              ///< Tracked groups
  std::uint16_t m_baseExpectedWkc{0};                       ///< Expected WKC for mandatory slaves
  std::size_t m_cycleCounter{0};                            ///< Scan cycles counter
  std::size_t m_scanInterval{DEFAULT_SCAN_INTERVAL_CYCLES}; ///< Scan interval in cycles

  /// @brief Probe and identify a disconnected Hot Connect group.
  bool probe_and_identify_group(HotConnectGroupConfig &group);
  /// @brief Initialize a newly identified Hot Connect group non-intrusively.
  bool initialize_group_non_intrusive(HotConnectGroupConfig &group);
  /// @brief Check health and WKC response of an active Hot Connect group.
  bool check_group_health(HotConnectGroupConfig &group);
};

} // namespace resoem::hot_connect
