#ifndef SIM_FIELD_HPP
#define SIM_FIELD_HPP

#include <Smp/IArrayField.h>
#include <Smp/IDataflowField.h>
#include <Smp/IField.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/IStructureField.h>
#include <core/Object.hpp>

namespace sim {

class Field : public core::Object, public virtual Smp::IField {
public:
  Field(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
        const Smp::Publication::IType *type, Smp::ViewKind view,
        Smp::Bool state, Smp::Bool input, Smp::Bool output);
  virtual ~Field() noexcept = default;

  Smp::ViewKind GetView() const override;
  Smp::Bool IsState() const override;
  Smp::Bool IsInput() const override;
  Smp::Bool IsOutput() const override;
  const Smp::Publication::IType *GetType() const override;

  // IPersist
  void Restore(Smp::IStorageReader *reader) override;
  void Store(Smp::IStorageWriter *writer) override;

private:
  const Smp::Publication::IType *_type;
  Smp::ViewKind _view;
  Smp::Bool _state;
  Smp::Bool _input;
  Smp::Bool _output;
};

class SimpleField : public Field, public virtual Smp::ISimpleField {
public:
  SimpleField(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
              const Smp::Publication::IType *type, void *address,
              Smp::ViewKind view, Smp::Bool state, Smp::Bool input,
              Smp::Bool output);

  Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const override;
  Smp::AnySimple GetValue() const override;
  void SetValue(Smp::AnySimple value) override;

private:
  void *_address;
};

} // namespace sim

#endif // SIM_FIELD_HPP
