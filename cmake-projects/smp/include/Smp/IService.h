#ifndef SMP_ISERVICE_H
#define SMP_ISERVICE_H

#include "Smp/ICollection.h"
#include "Smp/IComponent.h"

namespace Smp {
class IService : public virtual IComponent {
public:
  virtual ~IService() noexcept = default;
};

using ServiceCollection = ICollection<IService>;
} // namespace Smp

#endif // SMP_ISERVICE_H
