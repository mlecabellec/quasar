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
 */
class Field : public core::Object, public virtual Smp::IField {
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
  const Smp::Publication::IType *GetType() const override;

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
 */
class SimpleField : public Field, public virtual Smp::ISimpleField {
public:
  SimpleField(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
              const Smp::Publication::IType *type, void *address,
              Smp::ViewKind view, Smp::Bool state, Smp::Bool input,
              Smp::Bool output);

  /// [FE-0060.11.4] Return the primitive type kind.
  Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const override;
  /// [FE-0060.11.2] Return the field value.
  Smp::AnySimple GetValue() const override;
  /// [FE-0060.11.3] Set the field value.
  void SetValue(Smp::AnySimple value) override;

private:
  void *_address;
};

} // namespace sim

#endif // SIM_FIELD_HPP
