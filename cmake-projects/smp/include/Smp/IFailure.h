#ifndef SMP_IFAILURE_H
#define SMP_IFAILURE_H

#include "Smp/ICollection.h"
#include "Smp/IPersist.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IFailure : public virtual IPersist {
public:
  virtual ~IFailure() noexcept = default;

  virtual void Fail() = 0;
  virtual void Unfail() = 0;
  virtual Bool IsFailed() const = 0;
};

typedef ICollection<IFailure> FailureCollection;
} // namespace Smp

#endif // SMP_IFAILURE_H
