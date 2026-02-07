#ifndef SMP_PROPERTY_IMPL_H
#define SMP_PROPERTY_IMPL_H

#include "Smp/IProperty.h"
#include "Smp/Object.h"

namespace Smp {

class Property : public virtual IProperty, public Object {
public:
  Property(String8 name, String8 description, IObject *parent,
           Publication::IType *type, AccessKind access, ViewKind view)
      : Object(name, description, parent), type(type), access(access),
        view(view) {}

  virtual ~Property() noexcept = default;

  Publication::IType *GetType() const override { return type; }
  AccessKind GetAccess() const override { return access; }
  ViewKind GetView() const override { return view; }

  AnySimple GetValue() const override {
    // Concrete implementation would typically call a getter function or read
    // manual value
    return AnySimple();
  }

  void SetValue(AnySimple value) override {
    // Concrete implementation would typically call a setter function
  }

private:
  Publication::IType *type;
  AccessKind access;
  ViewKind view;
};

} // namespace Smp

#endif // SMP_PROPERTY_IMPL_H
