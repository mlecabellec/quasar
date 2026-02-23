#pragma once

#include <Smp/IComposite.h>
#include <Smp/IModel.h>
#include <Smp/ISimulator.h>
#include <core/Container.hpp>
#include <core/Object.hpp>
#include <core/SimpleCollection.hpp>

namespace sim {

/**
 * @brief Base Model implementation.
 * @details Contributes to [FE-0060.3.18] (IModel interface) and [FE-0060.5.1] (IComposite interface).
 */
class Model : public core::Object,
              public virtual Smp::IModel,
              public virtual Smp::IComposite {
public:
  Model(Smp::String8 name, Smp::String8 description = "",
        Smp::IObject *parent = nullptr, Smp::ISimulator *simulator = nullptr);
  virtual ~Model() noexcept = default;

  // IComposite methods
  /// [FE-0060.5.3] Return all containers.
  const Smp::ContainerCollection *GetContainers() const override;
  /// [FE-0060.5.2] Return a container by name.
  Smp::IContainer *GetContainer(Smp::String8 name) const override;

  // IComponent methods
  /// [FE-0060.3.2] Return current state of the component.
  Smp::ComponentStateKind GetState() const override;
  /// [FE-0060.3.4] Used to publish fields, properties and operations.
  void Publish(Smp::IPublication *receiver) override;
  /// [FE-0060.3.5] Used to perform initial configuration.
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override;
  /// [FE-0060.3.6] Allow connecting to the simulator environment.
  void Connect(Smp::ISimulator *simulator) override;
  /// [FE-0060.3.7] Disconnect the component.
  void Disconnect() override;

  /// [FE-0060.3.8] Provide access to fields.
  Smp::IField *GetField(Smp::String8 fullName) const override;
  /// [FE-0060.3.9] Return a collection of component fields.
  const Smp::FieldCollection *GetFields() const override;
  /// [FE-0060.3.10] Return the component UUID.
  const Smp::Uuid &GetUuid() const override;

  /// [FE-0060.3.11] Return a simple type value.
  Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;
  /// [FE-0060.3.12] Set a simple type value.
  void SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) override;
  /// [FE-0060.3.13] Return a simple array value.
  void GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimple *values,
                           Smp::UInt64 startIndex = 0) const override;
  /// [FE-0060.3.14] Set a simple array value.
  void SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimpleArray values,
                           Smp::UInt64 startIndex = 0) override;

  /// [FE-0060.3.15] Add a new child object.
  Smp::Bool AddChild(Smp::IObject *child,
                     const Smp::IObject *collection) override;
  /// [FE-0060.3.16] Remove a child object.
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::IObject *collection) override;
  /// [FE-0060.3.17] Check for child existence.
  Smp::IObject *
  IsChildInCollection(Smp::String8 child,
                      const Smp::IObject *collection) const override;

  Smp::IObject *GetChild(Smp::String8 name) const override;

  // IObject methods
  Smp::String8 GetName() const override;
  Smp::String8 GetDescription() const override;
  Smp::IObject *GetParent() const override;

protected:
  Smp::ComponentStateKind _state;
  Smp::ISimulator *_simulator; // Reference to simulator

  core::SimpleCollection<Smp::IContainer> _containers;
  core::SimpleCollection<Smp::IField> _fields;
};

} // namespace sim
