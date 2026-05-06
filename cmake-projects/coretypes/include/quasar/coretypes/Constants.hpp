#ifndef QUASAR_CORETYPES_CONSTANTS_HPP
#define QUASAR_CORETYPES_CONSTANTS_HPP

#include <chrono>
#include <cstdint>

namespace quasar::coretypes {

/**
 * @brief Global and module-specific constants for the Quasar project.
 * @details Adheres to [CS-0040] Strategic Constant Management.
 */

// --- Scripting & Service Timeouts ---
static constexpr std::chrono::seconds DEFAULT_LUA_METHOD_TIMEOUT{10};
static constexpr std::chrono::seconds DEFAULT_OPCUA_METHOD_TIMEOUT{5};
static constexpr std::chrono::milliseconds DEFAULT_MUTEX_TIMEOUT{100};
static constexpr std::chrono::milliseconds DEFAULT_SERVICE_CYCLE_TIME{10};

// --- EtherCAT Timing & Limits ---
static constexpr std::chrono::milliseconds EC_SLAVE_REACTION_TIME_MS{10};
static constexpr std::chrono::microseconds EC_SLAVE_REACTION_TIME_US{100};
static constexpr std::chrono::milliseconds EC_SDO_ACCESS_TIMEOUT_MS{100};
static constexpr uint64_t EC_FSM_MAX_ITERATIONS{1000000};
static constexpr uint16_t EC_SII_CAT_MAX_ITERATIONS{1000};
static constexpr uint16_t EC_SII_STR_MAX_ITERATIONS{256};
static constexpr uint16_t EC_SII_PDO_MAX_ITERATIONS{1000};
static constexpr uint16_t EC_SII_PDO_ENTRY_MAX_ITERATIONS{256};

// --- Buffer & Resource Limits ---
static constexpr size_t BIT_BUFFER_MAX_SAFE_SIZE{1000000};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_CONSTANTS_HPP
