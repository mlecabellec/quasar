#ifndef SMP_IENTRYPOINTPUBLISHER_H
#define SMP_IENTRYPOINTPUBLISHER_H

#include "Smp/IEntryPoint.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IEntryPointPublisher : public virtual IObject {
public:
  virtual ~IEntryPointPublisher() noexcept = default;

  virtual const EntryPointCollection *GetEntryPoints() const = 0;
  virtual IEntryPoint *GetEntryPoint(String8 name) const = 0;
};
} // namespace Smp

#endif // SMP_IENTRYPOINTPUBLISHER_H
