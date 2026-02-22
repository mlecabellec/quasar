#include "sim/TypeRegistry.hpp"
#include <Smp/Publication/IType.h>

namespace sim {

// Type Base
Type::Type(Smp::String8 name, Smp::String8 description, Smp::Uuid uuid,
           Smp::PrimitiveTypeKind primitiveTypeKind)
    : core::Object(name, description, nullptr), _uuid(uuid),
      _primitiveTypeKind(primitiveTypeKind) {}

Smp::PrimitiveTypeKind Type::GetPrimitiveTypeKind() const {
  return _primitiveTypeKind;
}
Smp::Uuid Type::GetUuid() const { return _uuid; }
Smp::String8 Type::GetName() const { return core::Object::GetName(); }
Smp::String8 Type::GetDescription() const {
  return core::Object::GetDescription();
}
Smp::IObject *Type::GetParent() const { return nullptr; }

// FloatType
FloatType::FloatType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
                     Smp::Float64 min, Smp::Float64 max, Smp::Bool minInc,
                     Smp::Bool maxInc, Smp::String8 unit,
                     Smp::PrimitiveTypeKind type)
    : Type(name, desc, uuid, type), _min(min), _max(max), _minInc(minInc),
      _maxInc(maxInc), _unit(unit) {}
Smp::Float64 FloatType::GetMinimum() const { return _min; }
Smp::Float64 FloatType::GetMaximum() const { return _max; }
Smp::Bool FloatType::IsMinInclusive() const { return _minInc; }
Smp::Bool FloatType::IsMaxInclusive() const { return _maxInc; }
Smp::String8 FloatType::GetUnit() const { return _unit; }

// IntegerType
IntegerType::IntegerType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
                         Smp::Int64 min, Smp::Int64 max, Smp::String8 unit,
                         Smp::PrimitiveTypeKind type)
    : Type(name, desc, uuid, type), _min(min), _max(max), _unit(unit) {}
Smp::Int64 IntegerType::GetMinimum() const { return _min; }
Smp::Int64 IntegerType::GetMaximum() const { return _max; }
Smp::String8 IntegerType::GetUnit() const { return _unit; }

// EnumerationType
EnumerationType::EnumerationType(Smp::String8 name, Smp::String8 desc,
                                 Smp::Uuid uuid, Smp::Int16 memSize)
    : Type(name, desc, uuid, Smp::PrimitiveTypeKind::PTK_Int32),
      _memorySize(memSize) {}
void EnumerationType::AddLiteral(Smp::String8 name, Smp::String8 description,
                                 Smp::Int32 value) { /* Stub */ }
Smp::Int16 EnumerationType::GetMemorySize() const { return _memorySize; }

// ArrayType
ArrayType::ArrayType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
                     Smp::Uuid itemTypeUuid, Smp::Int64 itemSize,
                     Smp::Int64 count, Smp::Bool simpleArray)
    : Type(name, desc, uuid, Smp::PrimitiveTypeKind::PTK_None),
      _itemTypeUuid(itemTypeUuid), _itemSize(itemSize), _count(count),
      _simpleArray(simpleArray) {}
Smp::UInt64 ArrayType::GetSize() const { return _itemSize * _count; }
const Smp::Publication::IType *ArrayType::GetItemType() const {
  return nullptr;
}

// StringType
StringType::StringType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
                       Smp::Int64 length)
    : Type(name, desc, uuid, Smp::PrimitiveTypeKind::PTK_String8),
      _length(length) {}
Smp::Int64 StringType::GetLength() const { return _length; }

// StructureType
StructureType::StructureType(Smp::String8 name, Smp::String8 desc,
                             Smp::Uuid uuid)
    : Type(name, desc, uuid, Smp::PrimitiveTypeKind::PTK_None) {}
void StructureType::AddField(Smp::String8 name, Smp::String8 description,
                             Smp::Uuid uuid, Smp::Int64 offset) { /* Stub */ }

// ClassType
ClassType::ClassType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
                     Smp::Uuid baseClassUuid)
    : Type(name, desc, uuid, Smp::PrimitiveTypeKind::PTK_None),
      _baseClassUuid(baseClassUuid) {}
Smp::Uuid ClassType::GetBaseClass() const { return _baseClassUuid; }
void ClassType::AddField(Smp::String8 name, Smp::String8 description,
                         Smp::Uuid uuid, Smp::Int64 offset) { /* Stub */ }

// TypeRegistry
TypeRegistry::TypeRegistry()
    : core::Object("TypeRegistry", "SMP Type Registry", nullptr) {}

Smp::String8 TypeRegistry::GetName() const { return core::Object::GetName(); }
Smp::String8 TypeRegistry::GetDescription() const {
  return core::Object::GetDescription();
}
Smp::IObject *TypeRegistry::GetParent() const { return nullptr; }

Smp::Publication::IType *
TypeRegistry::GetType(Smp::PrimitiveTypeKind type) const {
  auto it = _typesByKind.find(type);
  if (it != _typesByKind.end())
    return it->second;
  return nullptr;
}

Smp::Publication::IType *TypeRegistry::GetType(Smp::Uuid typeUuid) const {
  auto it = _typesByUuid.find(typeUuid);
  if (it != _typesByUuid.end())
    return it->second.get();
  return nullptr;
}

Smp::Publication::IType *TypeRegistry::AddFloatType(
    Smp::String8 name, Smp::String8 description, Smp::Uuid typeUuid,
    Smp::Float64 minimum, Smp::Float64 maximum, Smp::Bool minInclusive,
    Smp::Bool maxInclusive, Smp::String8 unit, Smp::PrimitiveTypeKind type) {
  _typesByUuid[typeUuid] =
      std::make_unique<FloatType>(name, description, typeUuid, minimum, maximum,
                                  minInclusive, maxInclusive, unit, type);
  _typesByKind[type] = _typesByUuid[typeUuid].get();
  return _typesByUuid[typeUuid].get();
}

Smp::Publication::IType *
TypeRegistry::AddIntegerType(Smp::String8 name, Smp::String8 description,
                             Smp::Uuid typeUuid, Smp::Int64 minimum,
                             Smp::Int64 maximum, Smp::String8 unit,
                             Smp::PrimitiveTypeKind type) {
  _typesByUuid[typeUuid] = std::make_unique<IntegerType>(
      name, description, typeUuid, minimum, maximum, unit, type);
  _typesByKind[type] = _typesByUuid[typeUuid].get();
  return _typesByUuid[typeUuid].get();
}

Smp::Publication::IEnumerationType *
TypeRegistry::AddEnumerationType(Smp::String8 name, Smp::String8 description,
                                 Smp::Uuid typeUuid, Smp::Int16 memorySize) {
  _typesByUuid[typeUuid] = std::make_unique<EnumerationType>(
      name, description, typeUuid, memorySize);
  return static_cast<Smp::Publication::IEnumerationType *>(
      _typesByUuid[typeUuid].get());
}

Smp::Publication::IArrayType *
TypeRegistry::AddArrayType(Smp::String8 name, Smp::String8 description,
                           Smp::Uuid typeUuid, Smp::Uuid itemTypeUuid,
                           Smp::Int64 itemSize, Smp::Int64 arrayCount,
                           Smp::Bool simpleArray) {
  _typesByUuid[typeUuid] =
      std::make_unique<ArrayType>(name, description, typeUuid, itemTypeUuid,
                                  itemSize, arrayCount, simpleArray);
  return static_cast<Smp::Publication::IArrayType *>(
      _typesByUuid[typeUuid].get());
}

Smp::Publication::IType *TypeRegistry::AddStringType(Smp::String8 name,
                                                     Smp::String8 description,
                                                     Smp::Uuid typeUuid,
                                                     Smp::Int64 length) {
  _typesByUuid[typeUuid] =
      std::make_unique<StringType>(name, description, typeUuid, length);
  return _typesByUuid[typeUuid].get();
}

Smp::Publication::IStructureType *
TypeRegistry::AddStructureType(Smp::String8 name, Smp::String8 description,
                               Smp::Uuid typeUuid) {
  _typesByUuid[typeUuid] =
      std::make_unique<StructureType>(name, description, typeUuid);
  return static_cast<Smp::Publication::IStructureType *>(
      _typesByUuid[typeUuid].get());
}

Smp::Publication::IClassType *
TypeRegistry::AddClassType(Smp::String8 name, Smp::String8 description,
                           Smp::Uuid typeUuid, Smp::Uuid baseClassUuid) {
  _typesByUuid[typeUuid] =
      std::make_unique<ClassType>(name, description, typeUuid, baseClassUuid);
  return static_cast<Smp::Publication::IClassType *>(
      _typesByUuid[typeUuid].get());
}

} // namespace sim
