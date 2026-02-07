#ifndef SMP_INVALIDANYTYPE_H
#define SMP_INVALIDANYTYPE_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidAnyType : public Exception {
public:
  InvalidAnyType(PrimitiveTypeKind invalidType, PrimitiveTypeKind expectedType,
                 const IObject *sender = nullptr)
      : Exception("InvalidAnyType", "Invalid Any Type", "Invalid type", sender),
        invalidType(invalidType), expectedType(expectedType) {}
  virtual ~InvalidAnyType() noexcept = default;

  PrimitiveTypeKind GetInvalidType() const noexcept { return invalidType; }
  PrimitiveTypeKind GetExpectedType() const noexcept { return expectedType; }

private:
  PrimitiveTypeKind invalidType;
  PrimitiveTypeKind expectedType;
};
} // namespace Smp

#endif // SMP_INVALIDANYTYPE_H
