#pragma once

#include <Smp/IPublication.h>
#include <Smp/Publication/IArrayType.h>
#include <Smp/Publication/IClassType.h>
#include <Smp/Publication/IEnumerationType.h>
#include <Smp/Publication/IStringType.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/Publication/IType.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <core/Object.hpp>
#include <map>
#include <memory>

// Basic Implementation of Types
namespace sim {

/**
 * @brief Type implementation.
 * @details Contributes to [FE-0070.10.12] (IType interface).
 */
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
  Smp::IObject *GetChild(Smp::String8 name) const override;

  // IType overrides
  /// [FE-0070.10.13] Publish new field of this type.
  Smp::IField *Publish(Smp::Publication::IPublishField *receiver,
                       Smp::String8 name, Smp::String8 description,
                       Smp::Void *address,
                       Smp::ViewKind view = Smp::ViewKind::VK_All,
                       Smp::Bool state = true, Smp::Bool input = false,
                       Smp::Bool output = false) override;

protected:
  Smp::Uuid _uuid;
  Smp::PrimitiveTypeKind _primitiveTypeKind;
};

class FloatType : public Type {
public:
  FloatType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
            Smp::Float64 min, Smp::Float64 max, Smp::Bool minInc,
            Smp::Bool maxInc, Smp::String8 unit, Smp::PrimitiveTypeKind type);

  Smp::Float64 GetMinimum() const;
  Smp::Float64 GetMaximum() const;
  Smp::Bool IsMinInclusive() const;
  Smp::Bool IsMaxInclusive() const;
  Smp::String8 GetUnit() const;

private:
  Smp::Float64 _min, _max;
  Smp::Bool _minInc, _maxInc;
  Smp::String8 _unit;
};

class IntegerType : public Type {
public:
  IntegerType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
              Smp::Int64 min, Smp::Int64 max, Smp::String8 unit,
              Smp::PrimitiveTypeKind type);

  Smp::Int64 GetMinimum() const;
  Smp::Int64 GetMaximum() const;
  Smp::String8 GetUnit() const;

private:
  Smp::Int64 _min, _max;
  Smp::String8 _unit;
};

// A generic class for other types that might just be stubs for now
class EnumerationType : public Type,
                        public virtual Smp::Publication::IEnumerationType {
public:
  EnumerationType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid);
  void AddLiteral(Smp::String8 name, Smp::String8 description,
                  Smp::Int32 value) override;

private:
};

class ArrayType : public Type, public virtual Smp::Publication::IArrayType {
public:
  ArrayType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
            Smp::Uuid itemTypeUuid, Smp::UInt64 itemSize, Smp::UInt64 count,
            Smp::Bool simpleArray);
  Smp::UInt64 GetSize() const override;
  const Smp::Publication::IType *GetItemType() const override;

private:
  Smp::Uuid _itemTypeUuid;
  Smp::UInt64 _itemSize;
  Smp::UInt64 _count;
  Smp::Bool _simpleArray;
};

class StringType : public Type, public virtual Smp::Publication::IStringType {
public:
  StringType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
             Smp::UInt64 length);
  Smp::UInt64 GetMaxLength() const override;

private:
  Smp::UInt64 _length;
};

class StructureType : public Type,
                      public virtual Smp::Publication::IStructureType {
public:
  StructureType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid);
  void AddField(Smp::String8 name, Smp::String8 description, Smp::Uuid uuid,
                Smp::UInt64 offset, Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true, Smp::Bool input = false,
                Smp::Bool output = false) override;
};

class ClassType : public Type, public virtual Smp::Publication::IClassType {
public:
  ClassType(Smp::String8 name, Smp::String8 desc, Smp::Uuid uuid,
            Smp::Uuid baseClassUuid);
  void AddField(Smp::String8 name, Smp::String8 description, Smp::Uuid uuid,
                Smp::UInt64 offset, Smp::ViewKind view = Smp::ViewKind::VK_All,
                Smp::Bool state = true, Smp::Bool input = false,
                Smp::Bool output = false) override;

private:
  Smp::Uuid _baseClassUuid;
};

/**
 * @brief Type Registry service implementation.
 * @details This service manages registered types within the simulator.
 * Fulfills [FE-0070.10.1] (ITypeRegistry Interface).
 */
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
  /**
   * @brief Returns a type by its UUID.
   * @param typeUuid The UUID of the type.
   * @return Smp::Publication::IType* The type or nullptr.
   * @details Fulfills [FE-0070.10.2] (ITypeRegistry::GetType).
   */
  Smp::Publication::IType *GetType(Smp::Uuid typeUuid) const override;
  /**
   * @brief Returns a primitive type by its kind.
   * @param typeKind The kind of the primitive type.
   * @return Smp::Publication::IType* The type or nullptr.
   * @details Fulfills [FE-0070.10.3] (ITypeRegistry::GetType).
   */
  Smp::Publication::IType *
  GetType(Smp::PrimitiveTypeKind typeKind) const override;

  /// [FE-0070.10.5] Add Float type.
  Smp::Publication::IType *
  AddFloatType(Smp::String8 name, Smp::String8 description, Smp::Uuid typeUuid,
               Smp::Float64 minimum, Smp::Float64 maximum,
               Smp::Bool minInclusive, Smp::Bool maxInclusive,
               Smp::String8 unit,
               Smp::PrimitiveTypeKind type =
                   Smp::PrimitiveTypeKind::PTK_Float64) override;

  /// [FE-0070.10.6] Add Integer type.
  Smp::Publication::IType *AddIntegerType(
      Smp::String8 name, Smp::String8 description, Smp::Uuid typeUuid,
      Smp::Int64 minimum, Smp::Int64 maximum, Smp::String8 unit,
      Smp::PrimitiveTypeKind type = Smp::PrimitiveTypeKind::PTK_Int32) override;

  /**
   * @brief Adds an enumeration type.
   * @param name The name.
   * @param description The description.
   * @param typeUuid The UUID.
   * @return Smp::Publication::IEnumerationType* The created type.
   * @details Fulfills [FE-0070.10.4] (ITypeRegistry::AddEnumerationType).
   */
  Smp::Publication::IEnumerationType *
  AddEnumerationType(Smp::String8 name, Smp::String8 description,
                     Smp::Uuid typeUuid) override;

  /// [FE-0070.10.8] Add Array type.
  Smp::Publication::IArrayType *
  AddArrayType(Smp::String8 name, Smp::String8 description, Smp::Uuid typeUuid,
               Smp::Uuid itemTypeUuid, Smp::UInt64 itemSize,
               Smp::UInt64 arrayCount, Smp::Bool simpleArray = false) override;

  /// [FE-0070.10.9] Add String type.
  Smp::Publication::IStringType *AddStringType(Smp::String8 name,
                                               Smp::String8 description,
                                               Smp::Uuid typeUuid,
                                               Smp::UInt64 length) override;

  /// [FE-0070.10.10] Add Structure type.
  Smp::Publication::IStructureType *
  AddStructureType(Smp::String8 name, Smp::String8 description,
                   Smp::Uuid typeUuid) override;

  /// [FE-0070.10.11] Add Class type.
  Smp::Publication::IClassType *AddClassType(Smp::String8 name,
                                             Smp::String8 description,
                                             Smp::Uuid typeUuid,
                                             Smp::Uuid baseClassUuid) override;

private:
  std::map<Smp::Uuid, std::unique_ptr<Smp::Publication::IType>> _typesByUuid;
  std::map<Smp::PrimitiveTypeKind, Smp::Publication::IType *> _typesByKind;
};

} // namespace sim
