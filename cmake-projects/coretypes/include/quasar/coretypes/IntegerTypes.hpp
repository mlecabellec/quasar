/**
 * @file IntegerTypes.hpp
 * @brief Type aliases for commonly used Integer types.
 */

#ifndef QUASAR_CORETYPES_INTEGERTYPES_HPP
#define QUASAR_CORETYPES_INTEGERTYPES_HPP

#include "quasar/coretypes/Integer.hpp"
#include <cstdint>

namespace quasar {
namespace coretypes {

/** @brief 8-bit signed integer wrapper. */
using Byte = Integer<int8_t>;
/** @brief 16-bit signed integer wrapper. */
using Short = Integer<int16_t>;
/** @brief 32-bit signed integer wrapper. */
using Int = Integer<int32_t>;
/** @brief 64-bit signed integer wrapper. */
using Long = Integer<int64_t>;

/** @brief 8-bit unsigned integer wrapper. */
using UByte = Integer<uint8_t>;
/** @brief 16-bit unsigned integer wrapper. */
using UShort = Integer<uint16_t>;
/** @brief 32-bit unsigned integer wrapper. */
using UInt = Integer<uint32_t>;
/** @brief 64-bit unsigned integer wrapper. */
using ULong = Integer<uint64_t>;

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_INTEGERTYPES_HPP
