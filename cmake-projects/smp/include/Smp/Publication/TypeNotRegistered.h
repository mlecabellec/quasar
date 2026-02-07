#ifndef SMP_PUBLICATION_TYPENOTREGISTERED_H
#define SMP_PUBLICATION_TYPENOTREGISTERED_H

#include "Smp/Exception.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Uuid.h"

namespace Smp {
namespace Publication {
class TypeNotRegistered : public Exception {
public:
  TypeNotRegistered(PrimitiveTypeKind type, const IObject *sender = nullptr)
      : Exception("TypeNotRegistered", "Type Not Registered",
                  "Type not registered", sender),
        type(type), hasUuid(false) {}
  TypeNotRegistered(Uuid uuid, const IObject *sender = nullptr)
      : Exception("TypeNotRegistered", "Type Not Registered",
                  "Type not registered", sender),
        uuid(uuid), hasUuid(true) {}
  virtual ~TypeNotRegistered() noexcept = default;

private:
  PrimitiveTypeKind type;
  Uuid uuid;
  bool hasUuid;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_TYPENOTREGISTERED_H
