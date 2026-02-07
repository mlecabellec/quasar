#ifndef SMP_PUBLICATION_ITYPEREGISTRY_H
#define SMP_PUBLICATION_ITYPEREGISTRY_H

#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/IArrayType.h"
#include "Smp/Publication/IClassType.h"
#include "Smp/Publication/IEnumerationType.h"
#include "Smp/Publication/IStructureType.h"
#include "Smp/Publication/IType.h"
#include "Smp/Publication/InvalidPrimitiveType.h"
#include "Smp/Publication/TypeAlreadyRegistered.h"
#include "Smp/Uuid.h"

namespace Smp {
namespace Publication {
class ITypeRegistry {
public:
  virtual ~ITypeRegistry() noexcept = default;

  virtual IType *GetType(PrimitiveTypeKind type) const = 0;
  virtual IType *GetType(Uuid typeUuid) const = 0;

  virtual IType *
  AddFloatType(String8 name, String8 description, Uuid typeUuid,
               Float64 minimum, Float64 maximum, Bool minInclusive,
               Bool maxInclusive, String8 unit,
               PrimitiveTypeKind type = PrimitiveTypeKind::PTK_Float64) = 0;
  virtual IType *
  AddIntegerType(String8 name, String8 description, Uuid typeUuid,
                 Int64 minimum, Int64 maximum, String8 unit,
                 PrimitiveTypeKind type = PrimitiveTypeKind::PTK_Int32) = 0;
  virtual IEnumerationType *AddEnumerationType(String8 name,
                                               String8 description,
                                               Uuid typeUuid,
                                               Int16 memorySize) = 0;
  virtual IArrayType *AddArrayType(String8 name, String8 description,
                                   Uuid typeUuid, Uuid itemTypeUuid,
                                   Int64 itemSize, Int64 arrayCount,
                                   Bool simpleArray = false) = 0;
  virtual IType *AddStringType(String8 name, String8 description, Uuid typeUuid,
                               Int64 length) = 0;
  virtual IStructureType *AddStructureType(String8 name, String8 description,
                                           Uuid typeUuid) = 0;
  virtual IClassType *AddClassType(String8 name, String8 description,
                                   Uuid typeUuid, Uuid baseClassUuid) = 0;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_ITYPEREGISTRY_H
