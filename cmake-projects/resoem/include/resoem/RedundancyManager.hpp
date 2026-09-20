/**
 * @file RedundancyManager.hpp
 * @brief EtherCAT Cable Redundancy loop topology engine with failover and healing.
 * @details Contributes to [TSK-20260920-001.5], [TSK-20260920-001.6], and [TSK-20260920-001.7].
 */

#pragma once

#include "resoem/IRawSocket.hpp"
#include "resoem/RedundancyConstants.hpp"
#include "resoem/common.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

namespace resoem::redundancy {

/**
 * @class RedundancyManager
 * @brief Orchestrates EtherCAT cable redundancy across dual network interfaces.
 * @details Monitors Working Counter (WKC) from primary transmission. If a cable break
 * occurs, bifurcates transmission by transmitting reverse frames on the secondary interface
 * and merging process data and working counters. Executes background probing datagrams to
 * safely heal the loop when connectivity is restored.
 */
class RedundancyManager {
public:
  /**
   * @brief Construct a RedundancyManager.
   * @param socket Transport socket (must support dual interface/redundancy).
   */
  explicit RedundancyManager(IRawSocket &socket);

  /**
   * @brief Virtual destructor.
   */
  virtual ~RedundancyManager() noexcept = default;

  // Prevent copying
  RedundancyManager(const RedundancyManager &) = delete;
  RedundancyManager &operator=(const RedundancyManager &) = delete;

  /**
   * @brief Get current redundancy engine state.
   * @return RedundancyState (CIRCULAR_NORMAL, BIFURCATED_BROKEN, etc.).
   */
  [[nodiscard]] RedundancyState state() const noexcept;

  /**
   * @brief Set the expected Working Counter (WKC) for the cyclic frame.
   * @param expected_wkc Expected WKC value.
   */
  void set_expected_wkc(std::uint16_t expected_wkc) noexcept;

  /**
   * @brief Get current expected WKC.
   * @return Expected WKC.
   */
  [[nodiscard]] std::uint16_t expected_wkc() const noexcept;

  /**
   * @brief Get total number of detected cable break events.
   * @return Break counter.
   */
  [[nodiscard]] std::size_t break_count() const noexcept;

  /**
   * @brief Get total number of successful healing events.
   * @return Healing counter.
   */
  [[nodiscard]] std::size_t heal_count() const noexcept;

  /**
   * @brief Set the cycle interval between background healing probes.
   * @param cycles Cycle interval.
   */
  void set_probe_interval_cycles(std::size_t cycles) noexcept;

  /**
   * @brief Perform cyclic frame transmission with redundancy failover and healing.
   * @param frame_data Frame buffer (sent and updated in-place with received data).
   * @param expected_wkc Expected WKC for this frame.
   * @return Total working counter or error.
   */
  [[nodiscard]] Result<std::uint16_t> exchange_cyclic_frame(std::span<byte> frame_data,
                                                            std::uint16_t expected_wkc);

  /**
   * @brief Manually trigger a background healing probe datagram on the secondary port.
   * @return True if loop was confirmed healed.
   */
  bool check_healing_probe();

private:
  mutable std::timed_mutex m_mutex;                                 ///< Concurrency guard
  IRawSocket &m_socket;                                            ///< Transport socket
  RedundancyState m_state{RedundancyState::CIRCULAR_NORMAL};       ///< State machine
  std::uint16_t m_expectedWkc{0};                                  ///< Expected total WKC
  std::size_t m_breakCount{0};                                     ///< Break events count
  std::size_t m_healCount{0};                                      ///< Heal events count
  std::size_t m_cycleCounter{0};                                   ///< Cyclic invocation counter
  std::size_t m_probeIntervalCycles{DEFAULT_PROBE_INTERVAL_CYCLES}; ///< Healing probe frequency
  std::size_t m_consecutiveHealSuccess{0};                         ///< Healing confirmation accumulator

  // Internal helper methods
  [[nodiscard]] static std::uint16_t extract_wkc_from_frame(std::span<const byte> frame) noexcept;
  static void update_wkc_in_frame(std::span<byte> frame, std::uint16_t wkc) noexcept;
  static void merge_datagram_payloads(std::span<byte> target_frame,
                                      std::span<const byte> secondary_frame,
                                      std::span<const byte> original_tx) noexcept;
};

} // namespace resoem::redundancy
