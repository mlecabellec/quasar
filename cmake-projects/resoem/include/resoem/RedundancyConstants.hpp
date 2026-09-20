/**
 * @file RedundancyConstants.hpp
 * @brief Constants and enums for EtherCAT cable redundancy (loop topology) management.
 * @details Contributes to [TSK-20260920-001.5] and [FE-0040.7].
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace resoem::redundancy {

/// Default cycle interval between background healing probes during bifurcated mode.
constexpr std::size_t DEFAULT_PROBE_INTERVAL_CYCLES = 20;

/// Number of consecutive successful probes required to confirm physical loop healing.
constexpr std::size_t DEFAULT_HEAL_CONFIRM_COUNT = 2;

/// Receive timeout for redundancy frames in milliseconds.
constexpr int REDUNDANCY_TIMEOUT_MS = 2;

/**
 * @enum RedundancyState
 * @brief State of the EtherCAT cable redundancy loop engine.
 */
enum class RedundancyState : std::uint8_t {
  DISABLED = 0,          ///< Redundancy disabled (single NIC mode)
  CIRCULAR_NORMAL = 1,   ///< Closed ring topology healthy, frames loop back
  BIFURCATED_BROKEN = 2, ///< Cable broken, dual transmission on Primary & Secondary
  HEALING_PROBING = 3    ///< Background probing link healing to confirm restoration
};

} // namespace resoem::redundancy
