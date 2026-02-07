#ifndef SMP_PUBLICATION_TYPEREGISTRY_IMPL_H
#define SMP_PUBLICATION_TYPEREGISTRY_IMPL_H

#include "Smp/Publication/ITypeRegistry.h"
#include "Smp/Publication/Type.h"
#include <map>
#include <memory>
#include <vector>

namespace Smp::Publication {

class TypeRegistry : public virtual ITypeRegistry {
public:
  TypeRegistry();
  virtual ~TypeRegistry() noexcept = default;

  IType *GetType(PrimitiveTypeKind type) const override;
  IType *GetType(Uuid typeUuid) const override;

  IType *AddFloatType(
      String8 name, String8 description, Uuid typeUuid, Float64 minimum,
      Float64 maximum, Bool minInclusive, Bool maxInclusive, String8 unit,
      PrimitiveTypeKind type = PrimitiveTypeKind::PTK_Float64) override;
  IType *AddIntegerType(
      String8 name, String8 description, Uuid typeUuid, Int64 minimum,
      Int64 maximum, String8 unit,
      PrimitiveTypeKind type = PrimitiveTypeKind::PTK_Int32) override;
  IEnumerationType *AddEnumerationType(String8 name, String8 description,
                                       Uuid typeUuid,
                                       Int16 memorySize) override;
  IArrayType *AddArrayType(String8 name, String8 description, Uuid typeUuid,
                           Uuid itemTypeUuid, Int64 itemSize, Int64 arrayCount,
                           Bool simpleArray = false) override;
  IType *AddStringType(String8 name, String8 description, Uuid typeUuid,
                       Int64 length) override;
  IStructureType *AddStructureType(String8 name, String8 description,
                                   Uuid typeUuid) override;
  IClassType *AddClassType(String8 name, String8 description, Uuid typeUuid,
                           Uuid baseClassUuid) override;

private:
  std::map<PrimitiveTypeKind, IType *> primitiveTypes;
  std::map<Uuid, std::unique_ptr<IType>> types;

  void RegisterPrimitive(PrimitiveTypeKind kind, String8 name, Uuid uuid);
};

} // namespace Smp::Publication

#endif // SMP_PUBLICATION_TYPEREGISTRY_IMPL_H
