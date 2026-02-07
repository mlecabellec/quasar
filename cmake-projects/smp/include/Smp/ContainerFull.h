#ifndef SMP_CONTAINERFULL_H
#define SMP_CONTAINERFULL_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class ContainerFull : public virtual Exception {
public:
  virtual ~ContainerFull() noexcept = default;

  virtual String8 GetContainerName() const noexcept = 0;
  virtual Int64 GetContainerSize() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_CONTAINERFULL_H
