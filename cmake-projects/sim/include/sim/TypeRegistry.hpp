#pragma once

#include <Smp/Publication/IFloatType.h>
#include <Smp/Publication/IIntegerType.h>
#include <Smp/Publication/IStringType.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <core/Object.hpp>
#include <map>
#include <memory>

// Basic Implementation of Types
namespace sim {

class Type : public core::Object, public virtual Smp::Publication::IType {
public:
  Type(Smp::String8 name, Smp::String8 description, Smp::Uuid uuid,
       Smp::PrimitiveTypeKind primitiveTypeKind);
  virtual ~Type() noexcept = default;

  Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const override;
  Smp::Uuid GetUuid() const override;

  // IObject overrides
  Smp::String8 GetName() const override;
  Smp::String8 GetDescription() const override;
  Smp::IObject *GetParent() const override;

protected:
  Smp::Uuid _uuid;
  Smp::PrimitiveTypeKind _primitiveTypeKind;
};

class FloatType : public Type, public virtual Smp::Publication::IFloatType {
public:
  FloatType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
            Smp::Float64 min, Smp::Float64 max, Smp::Bool minInc,
            Smp::Bool maxInc, Smp::String8 unit, Smp::PrimitiveTypeKind type);

  Smp::Float64 GetMinimum() const override;
  Smp::Float64 GetMaximum() const override;
  Smp::Bool IsMinInclusive() const override;
  Smp::Bool IsMaxInclusive() const override;
  Smp::String8 GetUnit() const override;

private:
  Smp::Float64 _min, _max;
  Smp::Bool _minInc, _maxInc;
  Smp::String8 _unit;
};

class IntegerType : public Type, public virtual Smp::Publication::IIntegerType {
public:
  IntegerType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
              Smp::Int64 min, Smp::Int64 max, Smp::String8 unit,
              Smp::PrimitiveTypeKind type);

  Smp::Int64 GetMinimum() const override;
  Smp::Int64 GetMaximum() const override;
  Smp::String8 GetUnit() const override;

private:
  Smp::Int64 _min, _max;
  Smp::String8 _unit;
};

// A generic class for other types that might just be stubs for now
class EnumerationType : public Type,
                        public virtual Smp::Publication::IEnumerationType {
public:
  EnumerationType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
                  Smp::Int16 memSize);
  void AddLiteral(Smp::String8 name, Smp::String8 description,
                  Smp::Int32 value) override;

private:
};

class ArrayType : public Type, public virtual Smp::Publication::IArrayType {
public:
  ArrayType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
            Smp::Uuid itemTypeUuid, Smp::Int64 itemSize, Smp::Int64 count,
            Smp::Bool simpleArray);
  Smp::UInt64 GetSize() const override;
  const Smp::Publication::IType *GetItemType() const override;

private:
  Smp::Uuid _itemTypeUuid;
  Smp::Int64 _itemSize;
  Smp::Int64 _count;
  Smp::Bool _simpleArray;
};

class StringType : public Type, public virtual Smp::Publication::IStringType {
public:
  StringType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
             Smp::Int64 length);
  Smp::Int64 GetLength() const override;

private:
  Smp::Int64 _length;
};

class StructureType : public Type,
                      public virtual Smp::Publication::IStructureType {
public:
  StructureType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid);
  void AddField(Smp::String8 name, Smp::String8 description, Smp::Uuid uuid,
                Smp::Int64 offset, Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true, Smp::Bool input = false,
                Smp::Bool output = false) override;
};

class ClassType : public Type, public virtual Smp::Publication::IClassType {
public:
  ClassType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
            Smp::Uuid baseClassUuid);
  void AddField(Smp::String8 name, Smp::String8 description, Smp::Uuid uuid,
                Smp::Int64 offset, Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true, Smp::Bool input = false,
                Smp::Bool output = false) override;

private:
  Smp::Uuid _baseClassUuid;
};

class TypeRegistry : public core::Object,
                     public virtual Smp::Publication::ITypeRegistry {
public:
  TypeRegistry();
  virtual ~TypeRegistry() noexcept = default;

  // IObject overrides
  Smp::String8 GetName() const override;
  Smp::String8 GetDescription() const override;
  Smp::IObject *GetParent() const override;

  // ITypeRegistry overrides
  Smp::Publication::IType *GetType(Smp::PrimitiveTypeKind type) const override;
  Smp::Publication::IType *GetType(Smp::Uuid typeUuid) const override;

  Smp::Publication::IType *
  AddFloatType(Smp::String8 name, Smp::String8 description, Smp::Uuid typeUuid,
               Smp::Float64 minimum, Smp::Float64 maximum,
               Smp::Bool minInclusive, Smp::Bool maxInclusive,
               Smp::String8 unit,
               Smp::PrimitiveTypeKind type =
                   Smp::PrimitiveTypeKind::PTK_Float64) override;

  Smp::Publication::IType *AddIntegerType(
      Smp::String8 name, Smp::String8 description, Smp::Uuid typeUuid,
      Smp::Int64 minimum, Smp::Int64 maximum, Smp::String8 unit,
      Smp::PrimitiveTypeKind type = Smp::PrimitiveTypeKind::PTK_Int32) override;

  Smp::Publication::IEnumerationType *
  AddEnumerationType(Smp::String8 name, Smp::String8 description,
                     Smp::Uuid typeUuid, Smp::Int16 memorySize) override;

  Smp::Publication::IArrayType *
  AddArrayType(Smp::String8 name, Smp::String8 description, Smp::Uuid typeUuid,
               Smp::Uuid itemTypeUuid, Smp::Int64 itemSize,
               Smp::Int64 arrayCount, Smp::Bool simpleArray = false) override;

  Smp::Publication::IType *AddStringType(Smp::String8 name,
                                         Smp::String8 description,
                                         Smp::Uuid typeUuid,
                                         Smp::Int64 length) override;

  Smp::Publication::IStructureType *
  AddStructureType(Smp::String8 name, Smp::String8 description,
                   Smp::Uuid typeUuid) override;

  Smp::Publication::IClassType *AddClassType(Smp::String8 name,
                                             Smp::String8 description,
                                             Smp::Uuid typeUuid,
                                             Smp::Uuid baseClassUuid) override;

private:
  std::map<Smp::Uuid, std::unique_ptr<Smp::Publication::IType>> _typesByUuid;
  std::map<Smp::PrimitiveTypeKind, Smp::Publication::IType *> _typesByKind;
};

} // namespace sim
