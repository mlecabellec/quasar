#ifndef SMP_SIMPLEFIELD_IMPL_H
#define SMP_SIMPLEFIELD_IMPL_H

#include "Smp/Field.h"
#include "Smp/ISimpleField.h"

namespace Smp {

class SimpleField : public virtual ISimpleField, public Field {
public:
  SimpleField(String8 name, String8 description, IObject *parent, void *address,
              ViewKind view, Bool state, Bool input, Bool output,
              const Publication::IType *type)
      : Field(name, description, parent, view, state, input, output, type),
        address(address) {}

  virtual ~SimpleField() noexcept = default;

  PrimitiveTypeKind GetPrimitiveTypeKind() const override {
    return type ? type->GetPrimitiveTypeKind() : PrimitiveTypeKind::PTK_None;
  }

  AnySimple GetValue() const override {
    if (!address || !type)
      return AnySimple();
    PrimitiveTypeKind kind = type->GetPrimitiveTypeKind();
    switch (kind) {
    case PrimitiveTypeKind::PTK_Bool:
      return AnySimple(kind, *static_cast<Bool *>(address));
    case PrimitiveTypeKind::PTK_Char8:
      return AnySimple(kind, *static_cast<Char8 *>(address));
    case PrimitiveTypeKind::PTK_Int8:
      return AnySimple(kind, *static_cast<Int8 *>(address));
    case PrimitiveTypeKind::PTK_UInt8:
      return AnySimple(kind, *static_cast<UInt8 *>(address));
    case PrimitiveTypeKind::PTK_Int16:
      return AnySimple(kind, *static_cast<Int16 *>(address));
    case PrimitiveTypeKind::PTK_UInt16:
      return AnySimple(kind, *static_cast<UInt16 *>(address));
    case PrimitiveTypeKind::PTK_Int32:
      return AnySimple(kind, *static_cast<Int32 *>(address));
    case PrimitiveTypeKind::PTK_UInt32:
      return AnySimple(kind, *static_cast<UInt32 *>(address));
    case PrimitiveTypeKind::PTK_Int64:
      return AnySimple(kind, *static_cast<Int64 *>(address));
    case PrimitiveTypeKind::PTK_UInt64:
      return AnySimple(kind, *static_cast<UInt64 *>(address));
    case PrimitiveTypeKind::PTK_Float32:
      return AnySimple(kind, *static_cast<Float32 *>(address));
    case PrimitiveTypeKind::PTK_Float64:
      return AnySimple(kind, *static_cast<Float64 *>(address));
    case PrimitiveTypeKind::PTK_Duration:
      return AnySimple(kind, *static_cast<Duration *>(address));
    case PrimitiveTypeKind::PTK_DateTime:
      return AnySimple(kind, *static_cast<DateTime *>(address));
    case PrimitiveTypeKind::PTK_String8:
      return AnySimple(kind, *static_cast<String8 *>(address));
    default:
      return AnySimple();
    }
  }

  void SetValue(AnySimple value) override {
    if (!address || !type)
      return;
    PrimitiveTypeKind kind = type->GetPrimitiveTypeKind();
    // Here we should check if value.GetType() matches kind, or perform
    // conversion
    switch (kind) {
    case PrimitiveTypeKind::PTK_Bool:
      *static_cast<Bool *>(address) = static_cast<Bool>(value);
      break;
    case PrimitiveTypeKind::PTK_Char8:
      *static_cast<Char8 *>(address) = static_cast<Char8>(value);
      break;
    case PrimitiveTypeKind::PTK_Int8:
      *static_cast<Int8 *>(address) = static_cast<Int8>(value);
      break;
    case PrimitiveTypeKind::PTK_UInt8:
      *static_cast<UInt8 *>(address) = static_cast<UInt8>(value);
      break;
    case PrimitiveTypeKind::PTK_Int16:
      *static_cast<Int16 *>(address) = static_cast<Int16>(value);
      break;
    case PrimitiveTypeKind::PTK_UInt16:
      *static_cast<UInt16 *>(address) = static_cast<UInt16>(value);
      break;
    case PrimitiveTypeKind::PTK_Int32:
      *static_cast<Int32 *>(address) = static_cast<Int32>(value);
      break;
    case PrimitiveTypeKind::PTK_UInt32:
      *static_cast<UInt32 *>(address) = static_cast<UInt32>(value);
      break;
    case PrimitiveTypeKind::PTK_Int64:
      *static_cast<Int64 *>(address) = static_cast<Int64>(value);
      break;
    case PrimitiveTypeKind::PTK_UInt64:
      *static_cast<UInt64 *>(address) = static_cast<UInt64>(value);
      break;
    case PrimitiveTypeKind::PTK_Float32:
      *static_cast<Float32 *>(address) = static_cast<Float32>(value);
      break;
    case PrimitiveTypeKind::PTK_Float64:
      *static_cast<Float64 *>(address) = static_cast<Float64>(value);
      break;
    case PrimitiveTypeKind::PTK_Duration:
      *static_cast<Duration *>(address) = static_cast<Duration>(value);
      break;
    case PrimitiveTypeKind::PTK_DateTime:
      *static_cast<DateTime *>(address) = static_cast<DateTime>(value);
      break;
    case PrimitiveTypeKind::PTK_String8:
      *static_cast<String8 *>(address) = static_cast<String8>(value);
      break;
    default:
      break;
    }
  }

private:
  void *address;
};

} // namespace Smp

#endif // SMP_SIMPLEFIELD_IMPL_H
