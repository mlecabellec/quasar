#include "Smp/AnySimple.h"
#include "Smp/InvalidAnyType.h"
#include "Smp/PrimitiveTypes.h"
#include <cstring>
#include <iomanip>

namespace Smp {

AnySimple::AnySimple() : type(PrimitiveTypeKind::PTK_None) {
  std::memset(&value, 0, sizeof(value));
}

AnySimple::AnySimple(PrimitiveTypeKind kind) : type(kind) {
  std::memset(&value, 0, sizeof(value));
}

AnySimple::AnySimple(const AnySimple &other)
    : type(other.type), value(other.value) {
  if (type == PrimitiveTypeKind::PTK_String8 && other.stringStorage) {
    size_t len = std::strlen(other.stringStorage.get());
    stringStorage = std::make_unique<char[]>(len + 1);
    std::strcpy(stringStorage.get(), other.stringStorage.get());
  }
}

AnySimple::AnySimple(AnySimple &&other)
    : type(other.type), value(other.value),
      stringStorage(std::move(other.stringStorage)) {
  other.type = PrimitiveTypeKind::PTK_None;
}

AnySimple &AnySimple::operator=(const AnySimple &other) {
  if (this != &other) {
    type = other.type;
    value = other.value;
    stringStorage.reset();
    if (type == PrimitiveTypeKind::PTK_String8 && other.stringStorage) {
      size_t len = std::strlen(other.stringStorage.get());
      stringStorage = std::make_unique<char[]>(len + 1);
      std::strcpy(stringStorage.get(), other.stringStorage.get());
    }
  }
  return *this;
}

AnySimple &AnySimple::operator=(AnySimple &&other) {
  if (this != &other) {
    type = other.type;
    value = other.value;
    stringStorage = std::move(other.stringStorage);
    other.type = PrimitiveTypeKind::PTK_None;
  }
  return *this;
}

AnySimple::~AnySimple() noexcept {}

void AnySimple::SetValue(PrimitiveTypeKind kind, Bool val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_Bool:
    value.boolValue = val;
    break;
  case PrimitiveTypeKind::PTK_UInt8:
    value.uInt8Value = val;
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    value.uInt16Value = val;
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    value.uInt32Value = val;
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    value.uInt64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int8:
    value.int8Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int16:
    value.int16Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int32:
    value.int32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    value.int64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    value.durationValue = val;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    value.dateTimeValue = val;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    value.float32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    value.float64Value = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_Bool);
  }
  type = kind;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, Char8 val) {
  if (kind != PrimitiveTypeKind::PTK_Char8)
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_Char8);
  type = kind;
  value.char8Value = val;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, String8 val) {
  if (kind != PrimitiveTypeKind::PTK_String8)
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_String8);
  stringStorage.reset();
  type = kind;
  if (val) {
    size_t len = std::strlen(val);
    stringStorage = std::make_unique<char[]>(len + 1);
    std::strcpy(stringStorage.get(), val);
  }
}

void AnySimple::SetValue(PrimitiveTypeKind kind, UInt8 val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt8:
    value.uInt8Value = val;
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    value.uInt16Value = val;
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    value.uInt32Value = val;
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    value.uInt64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int16:
    value.int16Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int32:
    value.int32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    value.int64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    value.durationValue = val;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    value.dateTimeValue = val;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    value.float32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    value.float64Value = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_UInt8);
  }
  type = kind;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, UInt16 val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt16:
    value.uInt16Value = val;
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    value.uInt32Value = val;
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    value.uInt64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int32:
    value.int32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    value.int64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    value.durationValue = val;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    value.dateTimeValue = val;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    value.float32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    value.float64Value = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_UInt16);
  }
  type = kind;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, UInt32 val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt32:
    value.uInt32Value = val;
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    value.uInt64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    value.int64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    value.durationValue = val;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    value.dateTimeValue = val;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    value.float64Value = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_UInt32);
  }
  type = kind;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, UInt64 val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt64:
    value.uInt64Value = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_UInt64);
  }
  type = kind;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, Int8 val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_Int8:
    value.int8Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int16:
    value.int16Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int32:
    value.int32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    value.int64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    value.durationValue = val;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    value.dateTimeValue = val;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    value.float32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    value.float64Value = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_Int8);
  }
  type = kind;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, Int16 val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_Int16:
    value.int16Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int32:
    value.int32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    value.int64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    value.durationValue = val;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    value.dateTimeValue = val;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    value.float32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    value.float64Value = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_Int16);
  }
  type = kind;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, Int32 val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_Int32:
    value.int32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    value.int64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    value.durationValue = val;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    value.dateTimeValue = val;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    value.float64Value = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_Int32);
  }
  type = kind;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, Int64 val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_Int64:
    value.int64Value = val;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    value.durationValue = val;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    value.dateTimeValue = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_Int64);
  }
  type = kind;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, Float32 val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_Float32:
    value.float32Value = val;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    value.float64Value = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_Float32);
  }
  type = kind;
}

void AnySimple::SetValue(PrimitiveTypeKind kind, Float64 val) {
  stringStorage.reset();
  switch (kind) {
  case PrimitiveTypeKind::PTK_Float64:
    value.float64Value = val;
    break;
  default:
    throw InvalidAnyType(kind, PrimitiveTypeKind::PTK_Float64);
  }
  type = kind;
}

AnySimple::operator Bool() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue;
  default:
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_Bool);
  }
}

AnySimple::operator Char8() const {
  if (type != PrimitiveTypeKind::PTK_Char8)
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_Char8);
  return value.char8Value;
}

AnySimple::operator String8() const {
  if (type != PrimitiveTypeKind::PTK_String8)
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_String8);
  return stringStorage.get();
}

AnySimple::operator UInt8() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_UInt8:
    return value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return static_cast<UInt8>(value.uInt16Value);
  case PrimitiveTypeKind::PTK_UInt32:
    return static_cast<UInt8>(value.uInt32Value);
  case PrimitiveTypeKind::PTK_UInt64:
    return static_cast<UInt8>(value.uInt64Value);
  case PrimitiveTypeKind::PTK_Int8:
    return static_cast<UInt8>(value.int8Value);
  case PrimitiveTypeKind::PTK_Int16:
    return static_cast<UInt8>(value.int16Value);
  case PrimitiveTypeKind::PTK_Int32:
    return static_cast<UInt8>(value.int32Value);
  case PrimitiveTypeKind::PTK_Int64:
    return static_cast<UInt8>(value.int64Value);
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue ? 1 : 0;
  case PrimitiveTypeKind::PTK_Float32:
    return static_cast<UInt8>(value.float32Value);
  case PrimitiveTypeKind::PTK_Float64:
    return static_cast<UInt8>(value.float64Value);
  case PrimitiveTypeKind::PTK_Duration:
    return static_cast<UInt8>(value.durationValue);
  case PrimitiveTypeKind::PTK_DateTime:
    return static_cast<UInt8>(value.dateTimeValue);
  default:
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_UInt8);
  }
}

AnySimple::operator UInt16() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_UInt16:
    return value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt8:
    return value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return static_cast<UInt16>(value.uInt32Value);
  case PrimitiveTypeKind::PTK_UInt64:
    return static_cast<UInt16>(value.uInt64Value);
  case PrimitiveTypeKind::PTK_Int16:
    return static_cast<UInt16>(value.int16Value);
  case PrimitiveTypeKind::PTK_Int32:
    return static_cast<UInt16>(value.int32Value);
  case PrimitiveTypeKind::PTK_Int64:
    return static_cast<UInt16>(value.int64Value);
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue ? 1 : 0;
  case PrimitiveTypeKind::PTK_Float32:
    return static_cast<UInt16>(value.float32Value);
  case PrimitiveTypeKind::PTK_Float64:
    return static_cast<UInt16>(value.float64Value);
  case PrimitiveTypeKind::PTK_Duration:
    return static_cast<UInt16>(value.durationValue);
  case PrimitiveTypeKind::PTK_DateTime:
    return static_cast<UInt16>(value.dateTimeValue);
  default:
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_UInt16);
  }
}

AnySimple::operator UInt32() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_UInt32:
    return value.uInt32Value;
  case PrimitiveTypeKind::PTK_UInt8:
    return value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt64:
    return static_cast<UInt32>(value.uInt64Value);
  case PrimitiveTypeKind::PTK_Int32:
    return static_cast<UInt32>(value.int32Value);
  case PrimitiveTypeKind::PTK_Int64:
    return static_cast<UInt32>(value.int64Value);
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue ? 1 : 0;
  case PrimitiveTypeKind::PTK_Float32:
    return static_cast<UInt32>(value.float32Value);
  case PrimitiveTypeKind::PTK_Float64:
    return static_cast<UInt32>(value.float64Value);
  case PrimitiveTypeKind::PTK_Duration:
    return static_cast<UInt32>(value.durationValue);
  case PrimitiveTypeKind::PTK_DateTime:
    return static_cast<UInt32>(value.dateTimeValue);
  default:
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_UInt32);
  }
}

AnySimple::operator UInt64() const {
  if (type != PrimitiveTypeKind::PTK_UInt64)
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_UInt64);
  return value.uInt64Value;
}

AnySimple::operator Int8() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int8:
    return value.int8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return static_cast<Int8>(value.int16Value);
  case PrimitiveTypeKind::PTK_Int32:
    return static_cast<Int8>(value.int32Value);
  case PrimitiveTypeKind::PTK_Int64:
    return static_cast<Int8>(value.int64Value);
  case PrimitiveTypeKind::PTK_UInt8:
    return static_cast<Int8>(value.uInt8Value);
  case PrimitiveTypeKind::PTK_UInt16:
    return static_cast<Int8>(value.uInt16Value);
  case PrimitiveTypeKind::PTK_UInt32:
    return static_cast<Int8>(value.uInt32Value);
  case PrimitiveTypeKind::PTK_UInt64:
    return static_cast<Int8>(value.uInt64Value);
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue ? 1 : 0;
  case PrimitiveTypeKind::PTK_Float32:
    return static_cast<Int8>(value.float32Value);
  case PrimitiveTypeKind::PTK_Float64:
    return static_cast<Int8>(value.float64Value);
  case PrimitiveTypeKind::PTK_Duration:
    return static_cast<Int8>(value.durationValue);
  case PrimitiveTypeKind::PTK_DateTime:
    return static_cast<Int8>(value.dateTimeValue);
  default:
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_Int8);
  }
}

AnySimple::operator Int16() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int16:
    return value.int16Value;
  case PrimitiveTypeKind::PTK_Int8:
    return value.int8Value;
  case PrimitiveTypeKind::PTK_Int32:
    return static_cast<Int16>(value.int32Value);
  case PrimitiveTypeKind::PTK_Int64:
    return static_cast<Int16>(value.int64Value);
  case PrimitiveTypeKind::PTK_UInt8:
    return value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return static_cast<Int16>(value.uInt16Value);
  case PrimitiveTypeKind::PTK_UInt32:
    return static_cast<Int16>(value.uInt32Value);
  case PrimitiveTypeKind::PTK_UInt64:
    return static_cast<Int16>(value.uInt64Value);
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue ? 1 : 0;
  case PrimitiveTypeKind::PTK_Float32:
    return static_cast<Int16>(value.float32Value);
  case PrimitiveTypeKind::PTK_Float64:
    return static_cast<Int16>(value.float64Value);
  case PrimitiveTypeKind::PTK_Duration:
    return static_cast<Int16>(value.durationValue);
  case PrimitiveTypeKind::PTK_DateTime:
    return static_cast<Int16>(value.dateTimeValue);
  default:
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_Int16);
  }
}

AnySimple::operator Int32() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int32:
    return value.int32Value;
  case PrimitiveTypeKind::PTK_Int8:
    return value.int8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return value.int16Value;
  case PrimitiveTypeKind::PTK_Int64:
    return static_cast<Int32>(value.int64Value);
  case PrimitiveTypeKind::PTK_UInt8:
    return value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return static_cast<Int32>(value.uInt32Value);
  case PrimitiveTypeKind::PTK_UInt64:
    return static_cast<Int32>(value.uInt64Value);
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue ? 1 : 0;
  case PrimitiveTypeKind::PTK_Float32:
    return static_cast<Int32>(value.float32Value);
  case PrimitiveTypeKind::PTK_Float64:
    return static_cast<Int32>(value.float64Value);
  case PrimitiveTypeKind::PTK_Duration:
    return static_cast<Int32>(value.durationValue);
  case PrimitiveTypeKind::PTK_DateTime:
    return static_cast<Int32>(value.dateTimeValue);
  default:
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_Int32);
  }
}

AnySimple::operator Int64() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int64:
  case PrimitiveTypeKind::PTK_Duration:
  case PrimitiveTypeKind::PTK_DateTime:
    return value.int64Value;
  case PrimitiveTypeKind::PTK_Int8:
    return value.int8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return value.int16Value;
  case PrimitiveTypeKind::PTK_Int32:
    return value.int32Value;
  case PrimitiveTypeKind::PTK_UInt8:
    return value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return value.uInt32Value;
  case PrimitiveTypeKind::PTK_UInt64:
    return static_cast<Int64>(value.uInt64Value);
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue ? 1 : 0;
  case PrimitiveTypeKind::PTK_Float32:
    return static_cast<Int64>(value.float32Value);
  case PrimitiveTypeKind::PTK_Float64:
    return static_cast<Int64>(value.float64Value);
  default:
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_Int64);
  }
}

AnySimple::operator Float32() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Float32:
    return value.float32Value;
  case PrimitiveTypeKind::PTK_Float64:
    return static_cast<Float32>(value.float64Value);
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue ? 1.0f : 0.0f;
  case PrimitiveTypeKind::PTK_Int8:
    return static_cast<Float32>(value.int8Value);
  case PrimitiveTypeKind::PTK_UInt8:
    return static_cast<Float32>(value.uInt8Value);
  case PrimitiveTypeKind::PTK_Int16:
    return static_cast<Float32>(value.int16Value);
  case PrimitiveTypeKind::PTK_UInt16:
    return static_cast<Float32>(value.uInt16Value);
  case PrimitiveTypeKind::PTK_Int32:
    return static_cast<Float32>(value.int32Value);
  case PrimitiveTypeKind::PTK_UInt32:
    return static_cast<Float32>(value.uInt32Value);
  case PrimitiveTypeKind::PTK_Int64:
    return static_cast<Float32>(value.int64Value);
  case PrimitiveTypeKind::PTK_UInt64:
    return static_cast<Float32>(value.uInt64Value);
  default:
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_Float32);
  }
}

AnySimple::operator Float64() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Float64:
    return value.float64Value;
  case PrimitiveTypeKind::PTK_Float32:
    return value.float32Value;
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue ? 1.0 : 0.0;
  case PrimitiveTypeKind::PTK_Int8:
    return static_cast<Float64>(value.int8Value);
  case PrimitiveTypeKind::PTK_UInt8:
    return static_cast<Float64>(value.uInt8Value);
  case PrimitiveTypeKind::PTK_Int16:
    return static_cast<Float64>(value.int16Value);
  case PrimitiveTypeKind::PTK_UInt16:
    return static_cast<Float64>(value.uInt16Value);
  case PrimitiveTypeKind::PTK_Int32:
    return static_cast<Float64>(value.int32Value);
  case PrimitiveTypeKind::PTK_UInt32:
    return static_cast<Float64>(value.uInt32Value);
  case PrimitiveTypeKind::PTK_Int64:
    return static_cast<Float64>(value.int64Value);
  case PrimitiveTypeKind::PTK_UInt64:
    return static_cast<Float64>(value.uInt64Value);
  default:
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_Float64);
  }
}

String8 AnySimple::MoveString() {
  if (type != PrimitiveTypeKind::PTK_String8)
    throw InvalidAnyType(type, PrimitiveTypeKind::PTK_String8);
  char *ptr = stringStorage.release();
  type = PrimitiveTypeKind::PTK_None;
  return ptr;
}

PrimitiveTypeKind AnySimple::GetType() const noexcept { return type; }

bool AnySimple::operator==(const AnySimple &other) const {
  if (type != other.type)
    return false;
  switch (type) {
  case PrimitiveTypeKind::PTK_None:
    return true;
  case PrimitiveTypeKind::PTK_Char8:
    return value.char8Value == other.value.char8Value;
  case PrimitiveTypeKind::PTK_Bool:
    return value.boolValue == other.value.boolValue;
  case PrimitiveTypeKind::PTK_Int8:
    return value.int8Value == other.value.int8Value;
  case PrimitiveTypeKind::PTK_UInt8:
    return value.uInt8Value == other.value.uInt8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return value.int16Value == other.value.int16Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return value.uInt16Value == other.value.uInt16Value;
  case PrimitiveTypeKind::PTK_Int32:
    return value.int32Value == other.value.int32Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return value.uInt32Value == other.value.uInt32Value;
  case PrimitiveTypeKind::PTK_Int64:
    return value.int64Value == other.value.int64Value;
  case PrimitiveTypeKind::PTK_UInt64:
    return value.uInt64Value == other.value.uInt64Value;
  case PrimitiveTypeKind::PTK_Float32:
    return value.float32Value == other.value.float32Value;
  case PrimitiveTypeKind::PTK_Float64:
    return value.float64Value == other.value.float64Value;
  case PrimitiveTypeKind::PTK_Duration:
    return value.durationValue == other.value.durationValue;
  case PrimitiveTypeKind::PTK_DateTime:
    return value.dateTimeValue == other.value.dateTimeValue;
  case PrimitiveTypeKind::PTK_String8:
    if (stringStorage && other.stringStorage)
      return std::strcmp(stringStorage.get(), other.stringStorage.get()) == 0;
    return stringStorage == other.stringStorage;
  default:
    return false;
  }
}

bool AnySimple::operator!=(const AnySimple &other) const {
  return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, const AnySimple &obj) {
  switch (obj.type) {
  case PrimitiveTypeKind::PTK_None:
    os << "None";
    break;
  case PrimitiveTypeKind::PTK_Char8:
    os << obj.value.char8Value;
    break;
  case PrimitiveTypeKind::PTK_Bool:
    os << (obj.value.boolValue ? "true" : "false");
    break;
  case PrimitiveTypeKind::PTK_Int8:
    os << static_cast<int>(obj.value.int8Value);
    break;
  case PrimitiveTypeKind::PTK_UInt8:
    os << static_cast<int>(obj.value.uInt8Value);
    break;
  case PrimitiveTypeKind::PTK_Int16:
    os << obj.value.int16Value;
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    os << obj.value.uInt16Value;
    break;
  case PrimitiveTypeKind::PTK_Int32:
    os << obj.value.int32Value;
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    os << obj.value.uInt32Value;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    os << obj.value.int64Value;
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    os << obj.value.uInt64Value;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    os << obj.value.float32Value;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    os << obj.value.float64Value;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    os << obj.value.durationValue;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    os << obj.value.dateTimeValue;
    break;
  case PrimitiveTypeKind::PTK_String8:
    if (obj.stringStorage)
      os << obj.stringStorage.get();
    else
      os << "(null)";
    break;
  default:
    os << "Unknown";
    break;
  }
  return os;
}

} // namespace Smp
