#ifndef SMP_SERVICES_IRESOLVER_H
#define SMP_SERVICES_IRESOLVER_H

#include "Smp/IComponent.h"
#include "Smp/IObject.h"
#include "Smp/IService.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
namespace Services {
class IResolver : public virtual IService {
public:
  static constexpr Char8 SMP_Resolver[] = "Resolver";

  virtual ~IResolver() noexcept = default;

  virtual IObject *ResolveAbsolute(String8 absolutePath) = 0;
  virtual IObject *ResolveRelative(String8 relativePath,
                                   const IComponent *sender) = 0;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_IRESOLVER_H
