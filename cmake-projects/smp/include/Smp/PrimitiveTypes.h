#ifndef SMP_PRIMITIVETYPES_H
#define SMP_PRIMITIVETYPES_H

#include "Smp/Uuid.h"
#include <climits>
#include <cstdint>
#include <iosfwd>

#if CHAR_BIT != 8
#error "Platform must support 8-bit char."
#endif

namespace Smp {
using Char8 = char;
using Bool = bool;
using Int8 = int8_t;
using UInt8 = uint8_t;
using Int16 = int16_t;
using UInt16 = uint16_t;
using Int32 = int32_t;
using UInt32 = uint32_t;
using Int64 = int64_t;
using UInt64 = uint64_t;
using Float32 = float;
using Float64 = double;

/// Duration in nanoseconds, relative time.
using Duration = int64_t;

/// Absolute time in nanoseconds, relative to 2000-01-01 12:00:00.
using DateTime = int64_t;

using String8 = const char *;

enum class PrimitiveTypeKind : Int32 {
  PTK_None,
  PTK_Char8,
  PTK_Bool,
  PTK_Int8,
  PTK_UInt8,
  PTK_Int16,
  PTK_UInt16,
  PTK_Int32,
  PTK_UInt32,
  PTK_Int64,
  PTK_UInt64,
  PTK_Float32,
  PTK_Float64,
  PTK_Duration,
  PTK_DateTime,
  PTK_String8
};

std::ostream &operator<<(std::ostream &os, PrimitiveTypeKind kind);

class Uuids {
public:
  static constexpr Uuid Uuid_Uuid = {
      0, 0, 0, {' ', ' ', ' ', ' ', 'U', 'u', 'i', 'd'}};
  static constexpr Uuid Uuid_Void = {
      0, 0, 0, {' ', ' ', ' ', ' ', 'V', 'o', 'i', 'd'}};
  static constexpr Uuid Uuid_Char8 = {
      0, 0, 0, {' ', ' ', ' ', 'C', 'h', 'a', 'r', '8'}};
  static constexpr Uuid Uuid_Bool = {
      0, 0, 0, {' ', ' ', ' ', ' ', 'B', 'o', 'o', 'l'}};
  static constexpr Uuid Uuid_Int8 = {
      0, 0, 0, {' ', ' ', ' ', ' ', 'I', 'n', 't', '8'}};
  static constexpr Uuid Uuid_UInt8 = {
      0, 0, 0, {' ', ' ', ' ', 'U', 'I', 'n', 't', '8'}};
  static constexpr Uuid Uuid_Int16 = {
      0, 0, 0, {' ', ' ', ' ', 'I', 'n', 't', '1', '6'}};
  static constexpr Uuid Uuid_UInt16 = {
      0, 0, 0, {' ', ' ', 'U', 'I', 'n', 't', '1', '6'}};
  static constexpr Uuid Uuid_Int32 = {
      0, 0, 0, {' ', ' ', ' ', 'I', 'n', 't', '3', '2'}};
  static constexpr Uuid Uuid_UInt32 = {
      0, 0, 0, {' ', ' ', 'U', 'I', 'n', 't', '3', '2'}};
  static constexpr Uuid Uuid_Int64 = {
      0, 0, 0, {' ', ' ', ' ', 'I', 'n', 't', '6', '4'}};
  static constexpr Uuid Uuid_UInt64 = {
      0, 0, 0, {' ', ' ', 'U', 'I', 'n', 't', '6', '4'}};
  static constexpr Uuid Uuid_Float32 = {
      0, 0, 0, {' ', 'F', 'l', 'o', 'a', 't', '3', '2'}};
  static constexpr Uuid Uuid_Float64 = {
      0, 0, 0, {' ', 'F', 'l', 'o', 'a', 't', '6', '4'}};
  static constexpr Uuid Uuid_Duration = {
      0, 0, 0, {'D', 'u', 'r', 'a', 't', 'i', 'o', 'n'}};
  static constexpr Uuid Uuid_DateTime = {
      0, 0, 0, {'D', 'a', 't', 'e', 'T', 'i', 'm', 'e'}};
  static constexpr Uuid Uuid_String8 = {
      0, 0, 0, {' ', 'S', 't', 'r', 'i', 'n', 'g', '8'}};

  static constexpr Uuid Uuid_PrimitiveTypeKind = {0xd55b0e31, 0xe618, 0x11dc,
                                                  0xab64, 0xbf8df6d7b83a};
  static constexpr Uuid Uuid_EventId = {0xd54589a4, 0xe618, 0x11dc, 0xab64,
                                        0xbf8df6d7b83a};
  static constexpr Uuid Uuid_LogMessageKind = {0xd543404f, 0xe618, 0x11dc,
                                               0xab64, 0xbf8df6d7b83a};
  static constexpr Uuid Uuid_TimeKind = {0xd54589a6, 0xe618, 0x11dc, 0xab64,
                                         0xbf8df6d7b83a};
  static constexpr Uuid Uuid_ViewKind = {0xd579e033, 0xe618, 0x11dc, 0xab64,
                                         0xbf8df6d7b83a};
  static constexpr Uuid Uuid_ParameterDirectionKind = {
      0x1b3641ad, 0xf0f0, 0x11dc, 0xb3ae, 0x77a8f1ab4ab6};
  static constexpr Uuid Uuid_ComponentStateKind = {0xd55d57c7, 0xe618, 0x11dc,
                                                   0xab64, 0xbf8df6d7b83a};
  static constexpr Uuid Uuid_AccessKind = {0xe7d5e125, 0xeb8a, 0x11dc, 0x8642,
                                           0xc38618fe0a20};
  static constexpr Uuid Uuid_SimulatorStateKind = {0xd56483dc, 0xe618, 0x11dc,
                                                   0xab64, 0xbf8df6d7b83a};
};
} // namespace Smp

#endif // SMP_PRIMITIVETYPES_H
