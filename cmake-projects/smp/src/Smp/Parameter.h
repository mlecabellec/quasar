#ifndef SMP_PARAMETER_IMPL_H
#define SMP_PARAMETER_IMPL_H

#include "Smp/IParameter.h"
#include "Smp/Object.h"

namespace Smp {

class Parameter : public virtual IParameter, public Object {
public:
  Parameter(String8 name, String8 description, IObject *parent,
            const Publication::IType *type,
            Publication::ParameterDirectionKind direction)
      : Object(name, description, parent), type(type), direction(direction) {}

  virtual ~Parameter() noexcept = default;

  Publication::IType *GetType() const override {
    return const_cast<Publication::IType *>(type);
  }
  Publication::ParameterDirectionKind GetDirection() const override {
    return direction;
  }

private:
  const Publication::IType *type;
  Publication::ParameterDirectionKind direction;
};

} // namespace Smp

#endif // SMP_PARAMETER_IMPL_H
