/**
 * @file IntegerTypes.hpp
 * @brief Type aliases for commonly used Integer types.
 */

#ifndef QUASAR_CORETYPES_INTEGERTYPES_HPP
#define QUASAR_CORETYPES_INTEGERTYPES_HPP

#include "quasar/coretypes/Integer.hpp"
#include <cstddef>
#include <cstdint>

namespace quasar {
namespace coretypes {

/** @brief 8-bit signed integer wrapper. 
 * Fulfills [FE-0050.1.3] Mapping for SMP "Int8" Primitive Type.
 */
using Byte = Integer<int8_t>;
/** @brief 16-bit signed integer wrapper. 
 * Fulfills [FE-0050.1.3] Mapping for SMP "Int16" Primitive Type.
 */
using Short = Integer<int16_t>;
/** @brief 32-bit signed integer wrapper. 
 * Fulfills [FE-0050.1.3] Mapping for SMP "Int32" Primitive Type.
 */
using Int = Integer<int32_t>;
/** @brief 64-bit signed integer wrapper. 
 * Fulfills [FE-0050.1.3] Mapping for SMP "Int64" Primitive Type.
 */
using Long = Integer<int64_t>;

/** @brief 8-bit unsigned integer wrapper. 
 * Fulfills [FE-0050.1.3] Mapping for SMP "UInt8" Primitive Type.
 */
using UByte = Integer<uint8_t>;
/** @brief 16-bit unsigned integer wrapper. 
 * Fulfills [FE-0050.1.3] Mapping for SMP "UInt16" Primitive Type.
 */
using UShort = Integer<uint16_t>;
/** @brief 32-bit unsigned integer wrapper. 
 * Fulfills [FE-0050.1.3] Mapping for SMP "UInt32" Primitive Type.
 */
using UInt = Integer<uint32_t>;
/** @brief 64-bit unsigned integer wrapper. 
 * Fulfills [FE-0050.1.3] Mapping for SMP "UInt64" Primitive Type.
 */
using ULong = Integer<uint64_t>;

/** @brief size_t integer wrapper. */
using Size = Integer<size_t>;

/** @brief ptrdiff_t integer wrapper. */
using PtrDiff = Integer<ptrdiff_t>;

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_INTEGERTYPES_HPP
