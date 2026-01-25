#ifndef QUASAR_CORETYPES_INTEGERTYPES_HPP
#define QUASAR_CORETYPES_INTEGERTYPES_HPP

#include "quasar/coretypes/Integer.hpp"
#include <cstdint>

namespace quasar {
namespace coretypes {

using Byte = Integer<int8_t>;
using Short = Integer<int16_t>;
using Int = Integer<int32_t>;
using Long = Integer<int64_t>;

using UByte = Integer<uint8_t>;
using UShort = Integer<uint16_t>;
using UInt = Integer<uint32_t>;
using ULong = Integer<uint64_t>;

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_INTEGERTYPES_HPP
