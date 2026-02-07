#ifndef SMP_SERVICES_ENTRYPOINTNOTSUBSCRIBED_H
#define SMP_SERVICES_ENTRYPOINTNOTSUBSCRIBED_H

#include "Smp/Exception.h"
#include "Smp/IEntryPoint.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
namespace Services {
class EntryPointNotSubscribed : public virtual Exception {
public:
  virtual ~EntryPointNotSubscribed() noexcept = default;

  virtual const IEntryPoint *GetEntryPoint() const noexcept = 0;
  virtual String8 GetEventName() const noexcept = 0;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_ENTRYPOINTNOTSUBSCRIBED_H
