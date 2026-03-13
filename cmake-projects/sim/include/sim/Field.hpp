/**
 * @file Field.hpp
 * @brief Defines sim::Field and sim::SimpleField classes.
 *
 * sim::Field implements the Smp::IField interface, providing basic information
 * about a field within a simulation model. sim::SimpleField extends Field to
 * handle fields of primitive types, interfacing with Smp::AnySimple for value
 * access.
 *
 * Contribution to FE-0030:
 * - [FE-0030.1.9] Reflection and introspection: `Field::GetType()` and
 *   `SimpleField::GetPrimitiveTypeKind()` provide runtime information about a
 *   field's type, supporting introspection.
 * - [FE-0030.1, FE-0030.2, FE-0030.3] Foundation for primitive types: By
 *   leveraging `Smp::AnySimple` via `GetValue()` and `SetValue()`, `SimpleField`
 *   enables the simulation framework to interact with primitive numerical and
 *   string data, forming a basis for operations required by FE-0030.
 * - [FE-0030.9] Const Correctness: Methods such as `GetView()`, `IsState()`,
 *   `IsInput()`, `IsOutput()`, `GetType()`, `GetPrimitiveTypeKind()`, and
 *   `GetValue()` are marked `const`, contributing to the requirement.
 *
 * Missing parts related to FE-0030:
 * - Does not implement `quasar::coretypes::Number` or `quasar::coretypes::String`
 *   classes directly.
 * - Does not provide arithmetic, bitwise, or safe operation methods.
 * - Does not handle Buffer or BitBuffer related functionalities.
 */
#ifndef SIM_FIELD_HPP
#define SIM_FIELD_HPP

#include <Smp/IArrayField.h>
#include <Smp/IField.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/IStructureField.h>
#include <core/Object.hpp>

namespace sim {

/**
 * @brief Basic Field implementation.
 * @details Contributes to [FE-0060.11.17] (IField interface).
 * This class provides basic field properties like view, state, input/output
 * flags, and type information. It serves as a base for specific field types.
 * Its `GetType()` method supports introspection capabilities relevant to FE-0030.1.9.
 */
class Field : public core::Object, public virtual Smp::IField {
// [FE-0050.1.1] Base class for fields, supporting primitive and user-defined types.
// [FE-0030.1.9] Provides introspection via GetType().
// [FE-0050.1.6] Implements IField interface.

public:
  Field(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
        const Smp::Publication::IType *type, Smp::ViewKind view,
        Smp::Bool state, Smp::Bool input, Smp::Bool output);
  virtual ~Field() noexcept = default;

  /// [FE-0060.11.18] Return the View Kind.
  Smp::ViewKind GetView() const override;
  /// [FE-0060.11.19] Return true if field is state.
  Smp::Bool IsState() const override;
  /// [FE-0060.11.20] Return true if field is input.
  Smp::Bool IsInput() const override;
  /// [FE-0060.11.21] Return true if field is output.
  Smp::Bool IsOutput() const override;
  /// [FE-0060.11.22] Return the field type.
  const Smp::Publication::IType *GetType() const override; // Supports introspection [FE-0030.1.9]

  // IPersist
  /// [FE-0060.9.3] Restore persisted data.
  void Restore(Smp::IStorageReader *reader) override;
  /// [FE-0060.9.4] Write persisted data.
  void Store(Smp::IStorageWriter *writer) override;

private:
  const Smp::Publication::IType *_type;
  Smp::ViewKind _view;
  Smp::Bool _state;
  Smp::Bool _input;
  Smp::Bool _output;
};

/**
 * @brief Simple Field implementation for primitive types.
 * @details Contributes to [FE-0060.11.1] (ISimpleField interface).
 * This class handles fields of primitive types, providing access to their values
 * via Smp::AnySimple. This is crucial for supporting operations on primitive
 * types as required by FE-0030.1, FE-0030.2, and FE-0030.3.
 */
class SimpleField : public Field, public virtual Smp::ISimpleField {
// [FE-0050.1.1] Implements ISimpleField for primitive types.
// [FE-0030.1.9] Provides introspection via GetPrimitiveTypeKind().
// [FE-0050.1.3], [FE-0050.1.4], [FE-0050.1.5] Supports primitive, Duration, and DateTime types via Smp::AnySimple.
// [FE-0050.1.6] Implements ISimpleField interface.

public:
  SimpleField(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
              const Smp::Publication::IType *type, void *address,
              Smp::ViewKind view, Smp::Bool state, Smp::Bool input,
              Smp::Bool output);

  /// [FE-0060.11.4] Return the primitive type kind. Supports introspection [FE-0030.1.9].
  Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const override;
  /// [FE-0060.11.2] Return the field value. Interacts with Smp::AnySimple,
  /// supporting primitive type access for FE-0030.1, FE-0030.2, FE-0030.3.
  Smp::AnySimple GetValue() const override;
  /// [FE-0060.11.3] Set the field value. Interacts with Smp::AnySimple,
  /// supporting primitive type modification for FE-0030.1, FE-0030.2, FE-0030.3.
  void SetValue(Smp::AnySimple value) override;

private:
  void *_address;
};

/**
 * @brief Simple Array Field implementation.
 */
class SimpleArrayField : public Field, public virtual Smp::ISimpleArrayField {
public:
  SimpleArrayField(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
                   const Smp::Publication::IType *type, void *address,
                   Smp::UInt64 size, Smp::PrimitiveTypeKind primitiveType,
                   Smp::ViewKind view, Smp::Bool state, Smp::Bool input,
                   Smp::Bool output);

  Smp::UInt64 GetSize() const override;
  Smp::AnySimple GetValue(Smp::UInt64 index) const override;
  void SetValue(Smp::UInt64 index, Smp::AnySimple value) override;
  void GetValues(Smp::UInt64 length, Smp::AnySimple *values, Smp::UInt64 startIndex = 0) const override;
  void SetValues(Smp::UInt64 length, Smp::AnySimpleArray values, Smp::UInt64 startIndex = 0) override;

private:
  void *_address;
  Smp::UInt64 _size;
  Smp::PrimitiveTypeKind _primitiveType;
};

} // namespace sim

#endif // SIM_FIELD_HPP
