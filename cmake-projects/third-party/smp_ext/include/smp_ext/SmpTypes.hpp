#pragma once

#include <Smp/PrimitiveTypes.h>

namespace smp_ext {

// Alias for compatibility if needed, but prefer standard identifiers
using SimpleTypeKind = Smp::PrimitiveTypeKind;

// Constants for PrimitiveTypeKind to match Simba/SmpExt style if necessary
constexpr Smp::PrimitiveTypeKind ST_None = Smp::PrimitiveTypeKind::PTK_None;
constexpr Smp::PrimitiveTypeKind ST_Bool = Smp::PrimitiveTypeKind::PTK_Bool;
constexpr Smp::PrimitiveTypeKind ST_Char8 = Smp::PrimitiveTypeKind::PTK_Char8;
constexpr Smp::PrimitiveTypeKind ST_Int8 = Smp::PrimitiveTypeKind::PTK_Int8;
constexpr Smp::PrimitiveTypeKind ST_UInt8 = Smp::PrimitiveTypeKind::PTK_UInt8;
constexpr Smp::PrimitiveTypeKind ST_Int16 = Smp::PrimitiveTypeKind::PTK_Int16;
constexpr Smp::PrimitiveTypeKind ST_UInt16 = Smp::PrimitiveTypeKind::PTK_UInt16;
constexpr Smp::PrimitiveTypeKind ST_Int32 = Smp::PrimitiveTypeKind::PTK_Int32;
constexpr Smp::PrimitiveTypeKind ST_UInt32 = Smp::PrimitiveTypeKind::PTK_UInt32;
constexpr Smp::PrimitiveTypeKind ST_Int64 = Smp::PrimitiveTypeKind::PTK_Int64;
constexpr Smp::PrimitiveTypeKind ST_UInt64 = Smp::PrimitiveTypeKind::PTK_UInt64;
constexpr Smp::PrimitiveTypeKind ST_Float32 =
    Smp::PrimitiveTypeKind::PTK_Float32;
constexpr Smp::PrimitiveTypeKind ST_Float64 =
    Smp::PrimitiveTypeKind::PTK_Float64;
constexpr Smp::PrimitiveTypeKind ST_DateTime =
    Smp::PrimitiveTypeKind::PTK_DateTime;
constexpr Smp::PrimitiveTypeKind ST_Duration =
    Smp::PrimitiveTypeKind::PTK_Duration;
constexpr Smp::PrimitiveTypeKind ST_String8 =
    Smp::PrimitiveTypeKind::PTK_String8;

} // namespace smp_ext
