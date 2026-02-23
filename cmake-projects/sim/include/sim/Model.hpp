#pragma once

#include <Smp/IComposite.h>
#include <Smp/IModel.h>
#include <Smp/ISimulator.h>
#include <core/Container.hpp>
#include <core/Object.hpp>
#include <core/SimpleCollection.hpp>

namespace sim {

/**
 * @brief Base class for simulation models.
 * @details This class implements the core interfaces for models within the SMP
 * ecosystem.
 * Fulfills [FE-0060.3.1] (IComponent Interface) and [FE-0060.3.18] (IModel
 * Interface).
 */
class Model : public core::Object,
              public virtual Smp::IModel,
              public virtual Smp::IComposite {
public:
  Model(Smp::String8 name, Smp::String8 description = "",
        Smp::IObject *parent = nullptr, Smp::ISimulator *simulator = nullptr);
  virtual ~Model() noexcept = default;

  // IComposite methods
  /**
   * @brief Returns the collection of containers within this composite.
   * @return const Smp::ContainerCollection* The containers.
   * @details Fulfills [FE-0060.5.2] (IComposite::GetContainers).
   */
  const Smp::ContainerCollection *GetContainers() const override;
  /**
   * @brief Returns a container by name.
   * @param name The name of the container.
   * @return Smp::IContainer* The container or nullptr.
   * @details Fulfills [FE-0060.5.3] (IComposite::GetContainer).
   */
  Smp::IContainer *GetContainer(Smp::String8 name) const override;

  // IComponent methods
  /**
   * @brief Returns the current state of the component.
   * @return Smp::ComponentStateKind The component state.
   * @details Fulfills [FE-0060.3.2] (IComponent::GetState).
   */
  Smp::ComponentStateKind GetState() const override;
  /**
   * @brief Publishes the model's fields and operations.
   * @param receiver The publication receiver.
   * @details Fulfills [FE-0060.3.4] (IComponent::Publish).
   */
  void Publish(Smp::IPublication *receiver) override;
  /**
   * @brief Configures the model.
   * @param logger The logger service.
   * @param linkRegistry The link registry service.
   * @details Fulfills [FE-0060.3.5] (IComponent::Configure).
   */
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override;
  /**
   * @brief Connects the model to the simulator.
   * @param simulator The simulator instance.
   * @details Fulfills [FE-0060.3.6] (IComponent::Connect).
   */
  void Connect(Smp::ISimulator *simulator) override;
  /**
   * @brief Disconnects the model.
   * @details Fulfills [FE-0060.3.7] (IComponent::Disconnect).
   */
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
                     const Smp::ICollectionBase *collection) override;
  /// [FE-0060.3.16] Remove a child object.
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::ICollectionBase *collection) override;
  /// [FE-0060.3.17] Check for child existence.
  Smp::IObject *
  IsChildInCollection(Smp::String8 child,
                      const Smp::ICollectionBase *collection) const override;

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
