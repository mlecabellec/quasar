#ifndef SMP_IPERSIST_H
#define SMP_IPERSIST_H

#include "Smp/CannotRestore.h"
#include "Smp/CannotStore.h"
#include "Smp/IObject.h"
#include "Smp/IStorageReader.h"
#include "Smp/IStorageWriter.h"

namespace Smp {
class IPersist : public virtual IObject {
public:
  virtual ~IPersist() noexcept = default;

  virtual void Restore(IStorageReader *reader) = 0;
  virtual void Store(IStorageWriter *writer) = 0;
};
} // namespace Smp

#endif // SMP_IPERSIST_H
