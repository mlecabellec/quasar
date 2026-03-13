/**
 * @file Field.cpp
 * @brief Implementation of sim::Field and sim::SimpleField classes.
 *
 * sim::Field implements the Smp::IField interface, providing basic information
 * about a field within a simulation model. sim::SimpleField extends Field to
 * handle fields of primitive types, interfacing with Smp::AnySimple for value
 * access.
 *
 * Contribution to FE-0030:
 * - [FE-0030.1.9] Reflection and introspection: `Field::GetType()` and
 *   `SimpleField::GetPrimitiveTypeKind()` provide runtime information about a
 *   field's type, supporting introspection.
 * - [FE-0030.1, FE-0030.2, FE-0030.3] Foundation for primitive types: By
 *   leveraging `Smp::AnySimple` via `GetValue()` and `SetValue()`, `SimpleField`
 *   enables the simulation framework to interact with primitive numerical and
 *   string data, forming a basis for operations required by FE-0030.
 * - [FE-0030.9] Const Correctness: Methods such as `GetView()`, `IsState()`,
 *   `IsInput()`, `IsOutput()`, `GetType()`, `GetPrimitiveTypeKind()`, and
 *   `GetValue()` are marked `const`, contributing to the requirement.
 * - [FE-0050.1.6] Simple Field: The `SimpleField` class directly implements the `Smp::ISimpleField` interface.
 *
 * Missing parts related to FE-0030:
 * - Does not implement `quasar::coretypes::Number` or `quasar::coretypes::String`
 *   classes directly.
 * - Does not provide arithmetic, bitwise, or safe operation methods.
 * - Does not handle Buffer or BitBuffer related functionalities.
 */
#include <Smp/Publication/IType.h>
#include <sim/Field.hpp>
#include <stdexcept>

namespace sim {

Field::Field(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
             const Smp::Publication::IType *type, Smp::ViewKind view,
             Smp::Bool state, Smp::Bool input, Smp::Bool output)
    : core::Object(name, description, parent), _type(type), _view(view),
      _state(state), _input(input), _output(output) {}

Smp::ViewKind Field::GetView() const { return _view; } // Const correct [FE-0030.9]
Smp::Bool Field::IsState() const { return _state; } // Const correct [FE-0030.9]
Smp::Bool Field::IsInput() const { return _input; } // Const correct [FE-0030.9]
Smp::Bool Field::IsOutput() const { return _output; } // Const correct [FE-0030.9]
const Smp::Publication::IType *Field::GetType() const { return _type; } // Const correct [FE-0030.9], supports introspection [FE-0030.1.9]

void Field::Restore(Smp::IStorageReader *reader) {
  // Basic restore logic - to be expanded if needed
}

void Field::Store(Smp::IStorageWriter *writer) {
  // Basic store logic - to be expanded if needed
}

// SimpleField
SimpleField::SimpleField(Smp::String8 name, Smp::String8 description,
                         Smp::IObject *parent,
                         const Smp::Publication::IType *type, void *address,
                         Smp::ViewKind view, Smp::Bool state, Smp::Bool input,
                         Smp::Bool output)
    : Field(name, description, parent, type, view, state, input, output),
      _address(address) {}

Smp::PrimitiveTypeKind SimpleField::GetPrimitiveTypeKind() const {
  // Const correct [FE-0030.9], supports introspection [FE-0030.1.9]
  if (GetType()) {
    return GetType()->GetPrimitiveTypeKind();
  }
  return Smp::PrimitiveTypeKind::PTK_None;
}

Smp::AnySimple SimpleField::GetValue() const {
  if (!_address)
    return Smp::AnySimple();

  Smp::PrimitiveTypeKind kind = GetPrimitiveTypeKind();
  switch (kind) {
    case Smp::PrimitiveTypeKind::PTK_Bool:
      return Smp::AnySimple(kind, *static_cast<Smp::Bool *>(_address));
    case Smp::PrimitiveTypeKind::PTK_Char8:
      return Smp::AnySimple(kind, *static_cast<Smp::Char8 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_Int8:
      return Smp::AnySimple(kind, *static_cast<Smp::Int8 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_Int16:
      return Smp::AnySimple(kind, *static_cast<Smp::Int16 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_Int32:
      return Smp::AnySimple(kind, *static_cast<Smp::Int32 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_Int64:
      return Smp::AnySimple(kind, *static_cast<Smp::Int64 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_UInt8:
      return Smp::AnySimple(kind, *static_cast<Smp::UInt8 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_UInt16:
      return Smp::AnySimple(kind, *static_cast<Smp::UInt16 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_UInt32:
      return Smp::AnySimple(kind, *static_cast<Smp::UInt32 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_UInt64:
      return Smp::AnySimple(kind, *static_cast<Smp::UInt64 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_Float32:
      return Smp::AnySimple(kind, *static_cast<Smp::Float32 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_Float64:
      return Smp::AnySimple(kind, *static_cast<Smp::Float64 *>(_address));
    case Smp::PrimitiveTypeKind::PTK_Duration:
      return Smp::AnySimple(kind, *static_cast<Smp::Duration *>(_address));
    case Smp::PrimitiveTypeKind::PTK_DateTime:
      return Smp::AnySimple(kind, *static_cast<Smp::DateTime *>(_address));
    default:
      return Smp::AnySimple();
  }
}

void SimpleField::SetValue(Smp::AnySimple value) {
  if (!_address)
    return;

  Smp::PrimitiveTypeKind kind = GetPrimitiveTypeKind();
  switch (kind) {
    case Smp::PrimitiveTypeKind::PTK_Bool:
      *static_cast<Smp::Bool *>(_address) = (Smp::Bool)value; break;
    case Smp::PrimitiveTypeKind::PTK_Char8:
      *static_cast<Smp::Char8 *>(_address) = (Smp::Char8)value; break;
    case Smp::PrimitiveTypeKind::PTK_Int8:
      *static_cast<Smp::Int8 *>(_address) = (Smp::Int8)value; break;
    case Smp::PrimitiveTypeKind::PTK_Int16:
      *static_cast<Smp::Int16 *>(_address) = (Smp::Int16)value; break;
    case Smp::PrimitiveTypeKind::PTK_Int32:
      *static_cast<Smp::Int32 *>(_address) = (Smp::Int32)value; break;
    case Smp::PrimitiveTypeKind::PTK_Int64:
      *static_cast<Smp::Int64 *>(_address) = (Smp::Int64)value; break;
    case Smp::PrimitiveTypeKind::PTK_UInt8:
      *static_cast<Smp::UInt8 *>(_address) = (Smp::UInt8)value; break;
    case Smp::PrimitiveTypeKind::PTK_UInt16:
      *static_cast<Smp::UInt16 *>(_address) = (Smp::UInt16)value; break;
    case Smp::PrimitiveTypeKind::PTK_UInt32:
      *static_cast<Smp::UInt32 *>(_address) = (Smp::UInt32)value; break;
    case Smp::PrimitiveTypeKind::PTK_UInt64:
      *static_cast<Smp::UInt64 *>(_address) = (Smp::UInt64)value; break;
    case Smp::PrimitiveTypeKind::PTK_Float32:
      *static_cast<Smp::Float32 *>(_address) = (Smp::Float32)value; break;
    case Smp::PrimitiveTypeKind::PTK_Float64:
      *static_cast<Smp::Float64 *>(_address) = (Smp::Float64)value; break;
    case Smp::PrimitiveTypeKind::PTK_Duration:
      *static_cast<Smp::Duration *>(_address) = (Smp::Duration)value; break;
    case Smp::PrimitiveTypeKind::PTK_DateTime:
      *static_cast<Smp::DateTime *>(_address) = (Smp::DateTime)value; break;
    default:
      break;
  }
}

// SimpleArrayField
SimpleArrayField::SimpleArrayField(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
                                   const Smp::Publication::IType *type, void *address,
                                   Smp::UInt64 size, Smp::PrimitiveTypeKind primitiveType,
                                   Smp::ViewKind view, Smp::Bool state, Smp::Bool input,
                                   Smp::Bool output)
    : Field(name, description, parent, type, view, state, input, output),
      _address(address), _size(size), _primitiveType(primitiveType) {}

Smp::UInt64 SimpleArrayField::GetSize() const { return _size; }

Smp::AnySimple SimpleArrayField::GetValue(Smp::UInt64 index) const {
    if (index >= _size) return Smp::AnySimple();
    
    char* ptr = static_cast<char*>(_address);
    switch (_primitiveType) {
        case Smp::PrimitiveTypeKind::PTK_Bool: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::Bool*>(ptr) + index));
        case Smp::PrimitiveTypeKind::PTK_Int8: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::Int8*>(ptr) + index));
        case Smp::PrimitiveTypeKind::PTK_Int16: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::Int16*>(ptr) + index));
        case Smp::PrimitiveTypeKind::PTK_Int32: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::Int32*>(ptr) + index));
        case Smp::PrimitiveTypeKind::PTK_Int64: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::Int64*>(ptr) + index));
        case Smp::PrimitiveTypeKind::PTK_UInt8: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::UInt8*>(ptr) + index));
        case Smp::PrimitiveTypeKind::PTK_UInt16: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::UInt16*>(ptr) + index));
        case Smp::PrimitiveTypeKind::PTK_UInt32: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::UInt32*>(ptr) + index));
        case Smp::PrimitiveTypeKind::PTK_UInt64: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::UInt64*>(ptr) + index));
        case Smp::PrimitiveTypeKind::PTK_Float32: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::Float32*>(ptr) + index));
        case Smp::PrimitiveTypeKind::PTK_Float64: return Smp::AnySimple(_primitiveType, *(reinterpret_cast<Smp::Float64*>(ptr) + index));
        default: return Smp::AnySimple();
    }
}

void SimpleArrayField::SetValue(Smp::UInt64 index, Smp::AnySimple value) {
    if (index >= _size) return;
    
    char* ptr = static_cast<char*>(_address);
    switch (_primitiveType) {
        case Smp::PrimitiveTypeKind::PTK_Bool: *(reinterpret_cast<Smp::Bool*>(ptr) + index) = (Smp::Bool)value; break;
        case Smp::PrimitiveTypeKind::PTK_Int8: *(reinterpret_cast<Smp::Int8*>(ptr) + index) = (Smp::Int8)value; break;
        case Smp::PrimitiveTypeKind::PTK_Int16: *(reinterpret_cast<Smp::Int16*>(ptr) + index) = (Smp::Int16)value; break;
        case Smp::PrimitiveTypeKind::PTK_Int32: *(reinterpret_cast<Smp::Int32*>(ptr) + index) = (Smp::Int32)value; break;
        case Smp::PrimitiveTypeKind::PTK_Int64: *(reinterpret_cast<Smp::Int64*>(ptr) + index) = (Smp::Int64)value; break;
        case Smp::PrimitiveTypeKind::PTK_UInt8: *(reinterpret_cast<Smp::UInt8*>(ptr) + index) = (Smp::UInt8)value; break;
        case Smp::PrimitiveTypeKind::PTK_UInt16: *(reinterpret_cast<Smp::UInt16*>(ptr) + index) = (Smp::UInt16)value; break;
        case Smp::PrimitiveTypeKind::PTK_UInt32: *(reinterpret_cast<Smp::UInt32*>(ptr) + index) = (Smp::UInt32)value; break;
        case Smp::PrimitiveTypeKind::PTK_UInt64: *(reinterpret_cast<Smp::UInt64*>(ptr) + index) = (Smp::UInt64)value; break;
        case Smp::PrimitiveTypeKind::PTK_Float32: *(reinterpret_cast<Smp::Float32*>(ptr) + index) = (Smp::Float32)value; break;
        case Smp::PrimitiveTypeKind::PTK_Float64: *(reinterpret_cast<Smp::Float64*>(ptr) + index) = (Smp::Float64)value; break;
        default: break;
    }
}

void SimpleArrayField::GetValues(Smp::UInt64 length, Smp::AnySimple *values, Smp::UInt64 startIndex) const {
    for (Smp::UInt64 i = 0; i < length; ++i) {
        values[i] = GetValue(startIndex + i);
    }
}

void SimpleArrayField::SetValues(Smp::UInt64 length, Smp::AnySimpleArray values, Smp::UInt64 startIndex) {
    for (Smp::UInt64 i = 0; i < length; ++i) {
        SetValue(startIndex + i, values[i]);
    }
}

} // namespace sim
