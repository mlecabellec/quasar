#include "Smp/Publication/TypeRegistry.h"
#include "Smp/Publication/Type.h"
#include "Smp/Publication/TypeAlreadyRegistered.h"
#include "Smp/Publication/TypeNotRegistered.h"
#include <stdexcept>

namespace Smp::Publication {

TypeRegistry::TypeRegistry() {
  RegisterPrimitive(PrimitiveTypeKind::PTK_None, "None", Uuids::Uuid_Void);
  RegisterPrimitive(PrimitiveTypeKind::PTK_Char8, "Char8", Uuids::Uuid_Char8);
  RegisterPrimitive(PrimitiveTypeKind::PTK_Bool, "Bool", Uuids::Uuid_Bool);
  RegisterPrimitive(PrimitiveTypeKind::PTK_Int8, "Int8", Uuids::Uuid_Int8);
  RegisterPrimitive(PrimitiveTypeKind::PTK_UInt8, "UInt8", Uuids::Uuid_UInt8);
  RegisterPrimitive(PrimitiveTypeKind::PTK_Int16, "Int16", Uuids::Uuid_Int16);
  RegisterPrimitive(PrimitiveTypeKind::PTK_UInt16, "UInt16",
                    Uuids::Uuid_UInt16);
  RegisterPrimitive(PrimitiveTypeKind::PTK_Int32, "Int32", Uuids::Uuid_Int32);
  RegisterPrimitive(PrimitiveTypeKind::PTK_UInt32, "UInt32",
                    Uuids::Uuid_UInt32);
  RegisterPrimitive(PrimitiveTypeKind::PTK_Int64, "Int64", Uuids::Uuid_Int64);
  RegisterPrimitive(PrimitiveTypeKind::PTK_UInt64, "UInt64",
                    Uuids::Uuid_UInt64);
  RegisterPrimitive(PrimitiveTypeKind::PTK_Float32, "Float32",
                    Uuids::Uuid_Float32);
  RegisterPrimitive(PrimitiveTypeKind::PTK_Float64, "Float64",
                    Uuids::Uuid_Float64);
  RegisterPrimitive(PrimitiveTypeKind::PTK_Duration, "Duration",
                    Uuids::Uuid_Duration);
  RegisterPrimitive(PrimitiveTypeKind::PTK_DateTime, "DateTime",
                    Uuids::Uuid_DateTime);
  RegisterPrimitive(PrimitiveTypeKind::PTK_String8, "String8",
                    Uuids::Uuid_String8);
}

void TypeRegistry::RegisterPrimitive(PrimitiveTypeKind kind, String8 name,
                                     Uuid uuid) {
  auto type = std::make_unique<Type>(name, "", uuid, kind);
  primitiveTypes[kind] = type.get();
  types[uuid] = std::move(type);
}

IType *TypeRegistry::GetType(PrimitiveTypeKind type) const {
  auto it = primitiveTypes.find(type);
  if (it != primitiveTypes.end()) {
    return it->second;
  }
  return nullptr;
}

IType *TypeRegistry::GetType(Uuid typeUuid) const {
  auto it = types.find(typeUuid);
  if (it != types.end()) {
    return it->second.get();
  }
  return nullptr;
}

IType *TypeRegistry::AddFloatType(String8 name, String8 description,
                                  Uuid typeUuid, Float64 minimum,
                                  Float64 maximum, Bool minInclusive,
                                  Bool maxInclusive, String8 unit,
                                  PrimitiveTypeKind type) {
  if (types.find(typeUuid) != types.end())
    throw TypeAlreadyRegistered(name, description, typeUuid);
  auto itype =
      std::make_unique<FloatType>(name, description, typeUuid, minimum, maximum,
                                  minInclusive, maxInclusive, unit, type);
  auto ptr = itype.get();
  types[typeUuid] = std::move(itype);
  return ptr;
}

IType *TypeRegistry::AddIntegerType(String8 name, String8 description,
                                    Uuid typeUuid, Int64 minimum, Int64 maximum,
                                    String8 unit, PrimitiveTypeKind type) {
  if (types.find(typeUuid) != types.end())
    throw TypeAlreadyRegistered(name, description, typeUuid);
  auto itype = std::make_unique<IntegerType>(name, description, typeUuid,
                                             minimum, maximum, unit, type);
  auto ptr = itype.get();
  types[typeUuid] = std::move(itype);
  return ptr;
}

IEnumerationType *TypeRegistry::AddEnumerationType(String8 name,
                                                   String8 description,
                                                   Uuid typeUuid,
                                                   Int16 memorySize) {
  if (types.find(typeUuid) != types.end())
    throw TypeAlreadyRegistered(name, description, typeUuid);
  auto itype = std::make_unique<EnumerationType>(name, description, typeUuid,
                                                 memorySize);
  auto ptr = itype.get();
  types[typeUuid] = std::move(itype);
  return ptr;
}

IArrayType *TypeRegistry::AddArrayType(String8 name, String8 description,
                                       Uuid typeUuid, Uuid itemTypeUuid,
                                       Int64 itemSize, Int64 arrayCount,
                                       Bool simpleArray) {
  if (types.find(typeUuid) != types.end())
    throw TypeAlreadyRegistered(name, description, typeUuid);
  auto itype =
      std::make_unique<ArrayType>(name, description, typeUuid, itemTypeUuid,
                                  itemSize, arrayCount, simpleArray);
  auto ptr = itype.get();
  types[typeUuid] = std::move(itype);
  return ptr;
}

IType *TypeRegistry::AddStringType(String8 name, String8 description,
                                   Uuid typeUuid, Int64 length) {
  if (types.find(typeUuid) != types.end())
    throw TypeAlreadyRegistered(name, description, typeUuid);
  auto itype = std::make_unique<Type>(name, description, typeUuid,
                                      PrimitiveTypeKind::PTK_String8);
  auto ptr = itype.get();
  types[typeUuid] = std::move(itype);
  return ptr;
}

IStructureType *TypeRegistry::AddStructureType(String8 name,
                                               String8 description,
                                               Uuid typeUuid) {
  if (types.find(typeUuid) != types.end())
    throw TypeAlreadyRegistered(name, description, typeUuid);
  auto itype = std::make_unique<StructureType>(name, description, typeUuid);
  auto ptr = itype.get();
  types[typeUuid] = std::move(itype);
  return ptr;
}

IClassType *TypeRegistry::AddClassType(String8 name, String8 description,
                                       Uuid typeUuid, Uuid baseClassUuid) {
  if (types.find(typeUuid) != types.end())
    throw TypeAlreadyRegistered(name, description, typeUuid);
  auto itype =
      std::make_unique<ClassType>(name, description, typeUuid, baseClassUuid);
  auto ptr = itype.get();
  types[typeUuid] = std::move(itype);
  return ptr;
}

} // namespace Smp::Publication
