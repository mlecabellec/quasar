#ifndef SMP_ANYSIMPLE_H
#define SMP_ANYSIMPLE_H

#include "Smp/PrimitiveTypes.h"
#include <iostream>

namespace Smp {
class AnySimple final {
public:
  AnySimple();
  explicit AnySimple(PrimitiveTypeKind kind);

  template <typename Type>
  AnySimple(PrimitiveTypeKind kind, Type value) : AnySimple(kind) {
    this->SetValue(kind, value);
  }

  AnySimple(const AnySimple &other);
  AnySimple(AnySimple &&other);
  AnySimple &operator=(const AnySimple &other);
  AnySimple &operator=(AnySimple &&other);
  ~AnySimple() noexcept;

  void SetValue(PrimitiveTypeKind kind, Bool value);
  void SetValue(PrimitiveTypeKind kind, Char8 value);
  void SetValue(PrimitiveTypeKind kind, String8 value);
  void SetValue(PrimitiveTypeKind kind, UInt8 value);
  void SetValue(PrimitiveTypeKind kind, UInt16 value);
  void SetValue(PrimitiveTypeKind kind, UInt32 value);
  void SetValue(PrimitiveTypeKind kind, UInt64 value);
  void SetValue(PrimitiveTypeKind kind, Int8 value);
  void SetValue(PrimitiveTypeKind kind, Int16 value);
  void SetValue(PrimitiveTypeKind kind, Int32 value);
  void SetValue(PrimitiveTypeKind kind, Int64 value);
  void SetValue(PrimitiveTypeKind kind, Float32 value);
  void SetValue(PrimitiveTypeKind kind, Float64 value);

  operator Bool() const;
  operator Char8() const;
  operator String8() const;
  operator UInt8() const;
  operator UInt16() const;
  operator UInt32() const;
  operator UInt64() const;
  operator Int8() const;
  operator Int16() const;
  operator Int32() const;
  operator Int64() const;
  operator Float32() const;
  operator Float64() const;

  String8 MoveString();
  PrimitiveTypeKind GetType() const noexcept;

  bool operator==(const AnySimple &other) const;
  bool operator!=(const AnySimple &other) const;

  union PrimitiveTypeValue {
    Char8 char8Value;
    Bool boolValue;
    Int8 int8Value;
    UInt8 uInt8Value;
    Int16 int16Value;
    UInt16 uInt16Value;
    Int32 int32Value;
    UInt32 uInt32Value;
    Int64 int64Value;
    UInt64 uInt64Value;
    Float32 float32Value;
    Float64 float64Value;
    Duration durationValue;
    DateTime dateTimeValue;
    String8 string8Value;
  };

  PrimitiveTypeKind type;
  PrimitiveTypeValue value;
};

std::ostream &operator<<(std::ostream &os, const AnySimple &obj);
} // namespace Smp

#endif // SMP_ANYSIMPLE_H
