#ifndef SMP_PUBLICATION_TYPEALREADYREGISTERED_H
#define SMP_PUBLICATION_TYPEALREADYREGISTERED_H

#include "Smp/Exception.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Uuid.h"

namespace Smp {
namespace Publication {
class IType;

class TypeAlreadyRegistered : public Exception {
public:
  TypeAlreadyRegistered(String8 name, String8 description, Uuid uuid,
                        const IObject *sender = nullptr)
      : Exception("TypeAlreadyRegistered", "Type Already Registered",
                  "Type already registered", sender),
        uuid(uuid) {}
  virtual ~TypeAlreadyRegistered() noexcept = default;

  Uuid GetUuid() const noexcept { return uuid; }

private:
  Uuid uuid;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_TYPEALREADYREGISTERED_H
