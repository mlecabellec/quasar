/**
 * @file FloatingPointTypes.hpp
 * @brief Type aliases for commonly used FloatingPoint types.
 */

#ifndef QUASAR_CORETYPES_FLOATINGPOINTTYPES_HPP
#define QUASAR_CORETYPES_FLOATINGPOINTTYPES_HPP

#include "quasar/coretypes/FloatingPoint.hpp"

/**
 * @brief Namespace containing all core type definitions in the Quasar framework.
 */
namespace quasar {
/**
 * @brief Namespace for fundamental numeric and buffer types.
 */
namespace coretypes {

/**
 * @brief 32-bit floating point wrapper.
 * Equivalent to Java's Float class.
 */
using Float = FloatingPoint<float>;

/**
 * @brief 64-bit floating point wrapper.
 * Equivalent to Java's Double class.
 */
using Double = FloatingPoint<double>;

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_FLOATINGPOINTTYPES_HPP
