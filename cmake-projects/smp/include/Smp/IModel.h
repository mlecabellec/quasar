#ifndef SMP_IMODEL_H
#define SMP_IMODEL_H

#include "Smp/ICollection.h"
#include "Smp/IComponent.h"

namespace Smp {
class IModel : public virtual IComponent {
public:
  virtual ~IModel() noexcept = default;
};

using ModelCollection = ICollection<IModel>;
} // namespace Smp

#endif // SMP_IMODEL_H
