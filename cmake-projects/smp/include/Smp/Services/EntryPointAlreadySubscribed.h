#ifndef SMP_SERVICES_ENTRYPOINTALREADYSUBSCRIBED_H
#define SMP_SERVICES_ENTRYPOINTALREADYSUBSCRIBED_H

#include "Smp/Exception.h"
#include "Smp/IEntryPoint.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
namespace Services {
class EntryPointAlreadySubscribed : public virtual Exception {
public:
  virtual ~EntryPointAlreadySubscribed() noexcept = default;

  virtual const IEntryPoint *GetEntryPoint() const noexcept = 0;
  virtual String8 GetEventName() const noexcept = 0;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_ENTRYPOINTALREADYSUBSCRIBED_H
