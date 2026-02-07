#include "Smp/PrimitiveTypes.h"
#include <iostream>

namespace Smp {
std::ostream &operator<<(std::ostream &os, PrimitiveTypeKind kind) {
  switch (kind) {
  case PrimitiveTypeKind::PTK_None:
    os << "None";
    break;
  case PrimitiveTypeKind::PTK_Char8:
    os << "Char8";
    break;
  case PrimitiveTypeKind::PTK_Bool:
    os << "Bool";
    break;
  case PrimitiveTypeKind::PTK_Int8:
    os << "Int8";
    break;
  case PrimitiveTypeKind::PTK_UInt8:
    os << "UInt8";
    break;
  case PrimitiveTypeKind::PTK_Int16:
    os << "Int16";
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    os << "UInt16";
    break;
  case PrimitiveTypeKind::PTK_Int32:
    os << "Int32";
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    os << "UInt32";
    break;
  case PrimitiveTypeKind::PTK_Int64:
    os << "Int64";
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    os << "UInt64";
    break;
  case PrimitiveTypeKind::PTK_Float32:
    os << "Float32";
    break;
  case PrimitiveTypeKind::PTK_Float64:
    os << "Float64";
    break;
  case PrimitiveTypeKind::PTK_Duration:
    os << "Duration";
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    os << "DateTime";
    break;
  case PrimitiveTypeKind::PTK_String8:
    os << "String8";
    break;
  default:
    os << "Unknown PrimitiveTypeKind (" << static_cast<int>(kind) << ")";
    break;
  }
  return os;
}
} // namespace Smp
