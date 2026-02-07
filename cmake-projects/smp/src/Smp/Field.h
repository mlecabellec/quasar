#ifndef SMP_FIELD_IMPL_H
#define SMP_FIELD_IMPL_H

#include "Smp/IField.h"
#include "Smp/Object.h"

namespace Smp {

class Field : public virtual IField, public Object {
public:
  Field(String8 name, String8 description, IObject *parent, ViewKind view,
        Bool state, Bool input, Bool output, const Publication::IType *type)
      : Object(name, description, parent), view(view), state(state),
        input(input), output(output), type(type) {}

  virtual ~Field() noexcept = default;

  ViewKind GetView() const override { return view; }
  Bool IsState() const override { return state; }
  Bool IsInput() const override { return input; }
  Bool IsOutput() const override { return output; }
  const Publication::IType *GetType() const override { return type; }

  void Restore(IStorageReader *reader) override {
    // Basic restoration logic for primitive types could be here if address is
    // known
  }

  void Store(IStorageWriter *writer) override {
    // Basic storage logic for primitive types could be here if address is known
  }

protected:
  ViewKind view;
  Bool state;
  Bool input;
  Bool output;
  const Publication::IType *type;
};

} // namespace Smp

#endif // SMP_FIELD_IMPL_H
