/**
 * @file HotConnectConstants.hpp
 * @brief Strategic constants and enumerations for EtherCAT Hot Connect subsystem.
 * @details Contributes to [TSK-20260920-001.8], [TSK-20260920-001.9], and [FE-0040.8].
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace resoem::hot_connect {

/**
 * @enum DisconnectedDataPolicy
 * @brief Behavior for input process data slots when a Hot Connect group is disconnected.
 */
enum class DisconnectedDataPolicy : std::uint8_t {
  ZERO_FILL = 0,       ///< Clear disconnected slave inputs to 0x00 (safe state)
  HOLD_LAST_STATE = 1,  ///< Preserve the last valid inputs received prior to disconnect
  CUSTOM_FALLBACK = 2   ///< Overwrite with pre-configured static fallback bytes
};

/**
 * @enum HotConnectGroupState
 * @brief Lifecycle state of a dynamic Hot Connect slave group.
 */
enum class HotConnectGroupState : std::uint8_t {
  DISCONNECTED = 0,    ///< Physical group is absent from bus
  DETECTED = 1,        ///< Identification command succeeded; group discovered
  INITIALIZING = 2,    ///< Performing non-intrusive ESM stepping (IP -> PS -> SO -> OP)
  OPERATIONAL = 3,     ///< Group is in OP and actively participating in cyclic exchange
  ERROR_STATE = 4      ///< Group failed initialization or validation
};

// Strategic Constants
constexpr std::size_t DEFAULT_SCAN_INTERVAL_CYCLES = 10;   ///< Hot connect scan period in cycles
constexpr int DEFAULT_IDENTIFY_TIMEOUT_MS = 50;            ///< Timeout for identification probe
constexpr std::size_t DEFAULT_IDENTIFY_RETRIES = 2;        ///< Retries for identification command
constexpr std::size_t MAX_HOT_CONNECT_GROUPS = 64;         ///< Maximum supported dynamic groups
constexpr std::size_t MAX_GROUP_SLAVES = 128;              ///< Maximum slaves per group
constexpr std::size_t MAX_FALLBACK_PAYLOAD_SIZE = 1024;    ///< Max fallback bytes buffer

} // namespace resoem::hot_connect
