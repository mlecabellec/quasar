#ifndef SMP_PUBLICATION_TYPE_H
#define SMP_PUBLICATION_TYPE_H

#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/IFloatType.h"
#include "Smp/Publication/IIntegerType.h"
#include "Smp/Publication/IStringType.h"
#include "Smp/Publication/IType.h"
#include <string>

namespace Smp::Publication {

class Type : public virtual IType {
public:
  Type(Smp::String8 name, Smp::String8 description, Uuid uuid,
       Smp::PrimitiveTypeKind kind)
      : name(name ? name : ""), description(description ? description : ""),
        uuid(uuid), kind(kind) {}

  virtual ~Type() noexcept = default;

  Smp::String8 GetName() const noexcept override { return name.c_str(); }
  Smp::String8 GetDescription() const noexcept override {
    return description.c_str();
  }
  IObject *GetParent() const noexcept override { return nullptr; }
  Uuid GetUuid() const noexcept override { return uuid; }
  Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const noexcept override {
    return kind;
  }

  void Publish(IPublication *receiver, Smp::String8 name,
               Smp::String8 description, void *address, ViewKind view,
               Smp::Bool state, Smp::Bool input, Smp::Bool output) override {
    // Basic types don't need complex publication logic,
    // They are handled directly by Publication::PublishField
  }

protected:
  std::string name;
  std::string description;
  Uuid uuid;
  Smp::PrimitiveTypeKind kind;
};

class IntegerType : public Type, public virtual IIntegerType {
public:
  IntegerType(Smp::String8 name, Smp::String8 description, Uuid uuid,
              Smp::Int64 min, Smp::Int64 max, Smp::String8 unit,
              Smp::PrimitiveTypeKind kind)
      : Type(name, description, uuid, kind), minimum(min), maximum(max),
        unit(unit ? unit : "") {}

  Smp::Int64 GetMinimum() const override { return minimum; }
  Smp::Int64 GetMaximum() const override { return maximum; }
  Smp::String8 GetUnit() const override { return unit.c_str(); }

  // Overriding IObject methods to avoid ambiguity
  Smp::String8 GetName() const noexcept override { return Type::GetName(); }
  Smp::String8 GetDescription() const noexcept override {
    return Type::GetDescription();
  }
  IObject *GetParent() const noexcept override { return Type::GetParent(); }

private:
  Smp::Int64 minimum;
  Smp::Int64 maximum;
  std::string unit;
};

class FloatType : public Type, public virtual IFloatType {
public:
  FloatType(Smp::String8 name, Smp::String8 description, Uuid uuid,
            Smp::Float64 min, Smp::Float64 max, Smp::Bool minInc,
            Smp::Bool maxInc, Smp::String8 unit, Smp::PrimitiveTypeKind kind)
      : Type(name, description, uuid, kind), minimum(min), maximum(max),
        minInclusive(minInc), maxInclusive(maxInc), unit(unit ? unit : "") {}

  Smp::Float64 GetMinimum() const override { return minimum; }
  Smp::Float64 GetMaximum() const override { return maximum; }
  Smp::Bool IsMinInclusive() const override { return minInclusive; }
  Smp::Bool IsMaxInclusive() const override { return maxInclusive; }
  Smp::String8 GetUnit() const override { return unit.c_str(); }

  // Overriding IObject methods to avoid ambiguity
  Smp::String8 GetName() const noexcept override { return Type::GetName(); }
  Smp::String8 GetDescription() const noexcept override {
    return Type::GetDescription();
  }
  IObject *GetParent() const noexcept override { return Type::GetParent(); }

private:
  Smp::Float64 minimum;
  Smp::Float64 maximum;
  Smp::Bool minInclusive;
  Smp::Bool maxInclusive;
  std::string unit;
};

class StringType : public Type, public virtual IStringType {
public:
  StringType(Smp::String8 name, Smp::String8 description, Uuid uuid,
             Smp::Int64 length)
      : Type(name, description, uuid, Smp::PrimitiveTypeKind::PTK_String8),
        length(length) {}

  Smp::Int64 GetLength() const override { return length; }

  // Overriding IObject methods to avoid ambiguity
  Smp::String8 GetName() const noexcept override { return Type::GetName(); }
  Smp::String8 GetDescription() const noexcept override {
    return Type::GetDescription();
  }
  IObject *GetParent() const noexcept override { return Type::GetParent(); }

private:
  Smp::Int64 length;
};

class ArrayType : public Type, public virtual IArrayType {
public:
  ArrayType(Smp::String8 name, Smp::String8 description, Uuid uuid,
            Uuid itemTypeUuid, Smp::Int64 itemSize, Smp::Int64 arrayCount,
            Smp::Bool simpleArray)
      : Type(name, description, uuid, Smp::PrimitiveTypeKind::PTK_None),
        itemTypeUuid(itemTypeUuid), itemSize(itemSize), arrayCount(arrayCount),
        simpleArray(simpleArray) {}

  Smp::UInt64 GetSize() const override {
    return static_cast<Smp::UInt64>(arrayCount);
  }
  const IType *GetItemType() const override {
    // This is problematic as we don't have access to TypeRegistry here
    // For now return nullptr or we might need to store the pointer if we
    // resolve it in TypeRegistry
    return nullptr;
  }

  // Overriding IObject methods
  Smp::String8 GetName() const noexcept override { return Type::GetName(); }
  Smp::String8 GetDescription() const noexcept override {
    return Type::GetDescription();
  }
  IObject *GetParent() const noexcept override { return Type::GetParent(); }

private:
  Uuid itemTypeUuid;
  Smp::Int64 itemSize;
  Smp::Int64 arrayCount;
  Smp::Bool simpleArray;
};

class EnumerationType : public Type, public virtual IEnumerationType {
public:
  struct Literal {
    std::string name;
    std::string description;
    Smp::Int32 value;
  };

  EnumerationType(Smp::String8 name, Smp::String8 description, Uuid uuid,
                  Smp::Int16 memorySize)
      : Type(name, description, uuid, Smp::PrimitiveTypeKind::PTK_Int32),
        memorySize(memorySize) {}

  void AddLiteral(Smp::String8 name, Smp::String8 description,
                  Smp::Int32 value) override {
    literals.push_back(
        {name ? name : "", description ? description : "", value});
  }

  // Overriding IObject methods
  Smp::String8 GetName() const noexcept override { return Type::GetName(); }
  Smp::String8 GetDescription() const noexcept override {
    return Type::GetDescription();
  }
  IObject *GetParent() const noexcept override { return Type::GetParent(); }

private:
  std::vector<Literal> literals;
  Smp::Int16 memorySize;
};

class StructureType : public Type, public virtual IStructureType {
public:
  struct FieldInfo {
    std::string name;
    std::string description;
    Uuid uuid;
    Smp::Int64 offset;
    ViewKind view;
    Smp::Bool state;
    Smp::Bool input;
    Smp::Bool output;
  };

  StructureType(Smp::String8 name, Smp::String8 description, Uuid uuid)
      : Type(name, description, uuid, Smp::PrimitiveTypeKind::PTK_None) {}

  void AddField(Smp::String8 name, Smp::String8 description, Uuid uuid,
                Smp::Int64 offset, ViewKind view = ViewKind::VK_All,
                Smp::Bool state = true, Smp::Bool input = false,
                Smp::Bool output = false) override {
    fields.push_back({name ? name : "", description ? description : "", uuid,
                      offset, view, state, input, output});
  }

  // Overriding IObject methods
  Smp::String8 GetName() const noexcept override { return Type::GetName(); }
  Smp::String8 GetDescription() const noexcept override {
    return Type::GetDescription();
  }
  IObject *GetParent() const noexcept override { return Type::GetParent(); }

protected:
  std::vector<FieldInfo> fields;
};

class ClassType : public StructureType, public virtual IClassType {
public:
  ClassType(Smp::String8 name, Smp::String8 description, Uuid uuid,
            Uuid baseClassUuid)
      : StructureType(name, description, uuid), baseClassUuid(baseClassUuid) {}

  // Overriding IObject methods
  Smp::String8 GetName() const noexcept override { return Type::GetName(); }
  Smp::String8 GetDescription() const noexcept override {
    return Type::GetDescription();
  }
  IObject *GetParent() const noexcept override { return Type::GetParent(); }

private:
  Uuid baseClassUuid;
};

} // namespace Smp::Publication

#endif // SMP_PUBLICATION_TYPE_H
