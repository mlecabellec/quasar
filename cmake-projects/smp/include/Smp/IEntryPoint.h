#ifndef SMP_IENTRYPOINT_H
#define SMP_IENTRYPOINT_H

#include "Smp/ICollection.h"
#include "Smp/IObject.h"

namespace Smp {
class IEntryPoint : public virtual IObject {
public:
  virtual ~IEntryPoint() noexcept = default;

  virtual void Execute() const = 0;
};

using EntryPointCollection = ICollection<IEntryPoint>;
} // namespace Smp

#endif // SMP_IENTRYPOINT_H
