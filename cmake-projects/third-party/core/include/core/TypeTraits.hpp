#pragma once

#include <cstdint>

namespace core {

/**
 * Type of data.
 */
enum class DataType {
  None,
  Char8,
  Int8,
  UInt8,
  Int16,
  UInt16,
  Int32,
  UInt32,
  Int64,
  UInt64,
  Float32,
  Float64,
  Bool,
  String,
  Object
};

} // namespace core
