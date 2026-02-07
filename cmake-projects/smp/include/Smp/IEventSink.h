#ifndef SMP_IEVENTSINK_H
#define SMP_IEVENTSINK_H

#include "Smp/AnySimple.h"
#include "Smp/ICollection.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IEventSink : public virtual IObject {
public:
  virtual ~IEventSink() noexcept = default;

  virtual PrimitiveTypeKind GetEventArgType() const = 0;
  virtual void Notify(IObject *sender, AnySimple arg) = 0;
};

typedef ICollection<IEventSink> EventSinkCollection;
} // namespace Smp

#endif // SMP_IEVENTSINK_H
