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
  // Const correct [FE-0030.9]. Interacts with Smp::AnySimple, supporting primitive
  // type access for FE-0030.1, FE-0030.2, FE-0030.3.
  if (!_address)
    return Smp::AnySimple();

  if (GetPrimitiveTypeKind() == Smp::PrimitiveTypeKind::PTK_Bool) {
    return Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool,
                          *static_cast<Smp::Bool *>(_address));
  }
  // TODO: Implement other types to fully support all primitive types
  // needed by FE-0030.1, FE-0030.2, FE-0030.3.
  return Smp::AnySimple();
}

void SimpleField::SetValue(Smp::AnySimple value) {
  // Interacts with Smp::AnySimple, supporting primitive type modification
  // for FE-0030.1, FE-0030.2, FE-0030.3.
  if (!_address)
    return;

  if (GetPrimitiveTypeKind() == Smp::PrimitiveTypeKind::PTK_Bool) {
    *static_cast<Smp::Bool *>(_address) = (Smp::Bool)value;
  }
  // TODO: Implement other types to fully support all primitive types
  // needed by FE-0030.1, FE-0030.2, FE-0030.3.
}

} // namespace sim
