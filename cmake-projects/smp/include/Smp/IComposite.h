#ifndef SMP_ICOMPOSITE_H
#define SMP_ICOMPOSITE_H

#include "Smp/IContainer.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IComposite : public virtual IObject {
public:
  virtual ~IComposite() noexcept = default;

  virtual const ContainerCollection *GetContainers() const = 0;
  virtual IContainer *GetContainer(String8 name) const = 0;
};
} // namespace Smp

#endif // SMP_ICOMPOSITE_H
