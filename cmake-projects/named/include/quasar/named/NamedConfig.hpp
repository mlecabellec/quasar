/**
 * @file NamedConfig.hpp
 * @brief Configuration constants and limits for the named hierarchy.
 */

#ifndef QUASAR_NAMED_NAMEDCONFIG_HPP
#define QUASAR_NAMED_NAMEDCONFIG_HPP

#include <cstddef>
#include <chrono>

namespace quasar::named {

/**
 * @namespace quasar::named::config
 * @brief Namespace for configuration constants.
 */
namespace config {

/**
 * @brief Default hard limit for loop iterations to prevent infinite loops.
 * @compliance [CS-0010.37] Hard limit for loops.
 */
inline constexpr std::size_t HARD_LIMIT_ITERATIONS = 1000000;

/**
 * @brief Default timeout for mutex locking.
 * @compliance [CS-0010.26] Use mutex with timeout.
 */
inline constexpr std::chrono::milliseconds DEFAULT_LOCK_TIMEOUT = std::chrono::milliseconds(5000);

} // namespace config

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDCONFIG_HPP
