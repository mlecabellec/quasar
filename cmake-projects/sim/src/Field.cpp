#include <Smp/Publication/IType.h>
#include <sim/Field.hpp>
#include <stdexcept>

namespace sim {

Field::Field(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
             const Smp::Publication::IType *type, Smp::ViewKind view,
             Smp::Bool state, Smp::Bool input, Smp::Bool output)
    : core::Object(name, description, parent), _type(type), _view(view),
      _state(state), _input(input), _output(output) {}

Smp::ViewKind Field::GetView() const { return _view; }
Smp::Bool Field::IsState() const { return _state; }
Smp::Bool Field::IsInput() const { return _input; }
Smp::Bool Field::IsOutput() const { return _output; }
const Smp::Publication::IType *Field::GetType() const { return _type; }

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
  if (GetType()) {
    return GetType()->GetPrimitiveTypeKind();
  }
  return Smp::PrimitiveTypeKind::PTK_None;
}

Smp::AnySimple SimpleField::GetValue() const {
  if (!_address)
    return Smp::AnySimple();

  if (GetPrimitiveTypeKind() == Smp::PrimitiveTypeKind::PTK_Bool) {
    return Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool,
                          *static_cast<Smp::Bool *>(_address));
  }
  // TODO: Implement other types
  return Smp::AnySimple();
}

void SimpleField::SetValue(Smp::AnySimple value) {
  if (!_address)
    return;

  if (GetPrimitiveTypeKind() == Smp::PrimitiveTypeKind::PTK_Bool) {
    *static_cast<Smp::Bool *>(_address) = (Smp::Bool)value;
  }
  // TODO: Implement other types
}

} // namespace sim
