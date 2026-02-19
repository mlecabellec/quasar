#pragma once

#include <Smp/IComposite.h>
#include <Smp/IModel.h>
#include <Smp/ISimulator.h>
#include <core/Container.hpp>
#include <core/Object.hpp>
#include <core/SimpleCollection.hpp>

namespace sim {

class Model : public core::Object,
              public virtual Smp::IModel,
              public virtual Smp::IComposite {
public:
  Model(Smp::String8 name, Smp::String8 description = "",
        Smp::IObject *parent = nullptr, Smp::ISimulator *simulator = nullptr);
  virtual ~Model() noexcept = default;

  // IComposite methods
  const Smp::ContainerCollection *GetContainers() const override;
  Smp::IContainer *GetContainer(Smp::String8 name) const override;

  // IComponent methods
  Smp::ComponentStateKind GetState() const override;
  void Publish(Smp::IPublication *receiver) override;
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override;
  void Connect(Smp::ISimulator *simulator) override;
  void Disconnect() override;

  Smp::IField *GetField(Smp::String8 fullName) const override;
  const Smp::FieldCollection *GetFields() const override;
  const Smp::Uuid &GetUuid() const override;

  Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;
  void SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) override;
  void GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimple *values,
                           Smp::UInt64 startIndex = 0) const override;
  void SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimpleArray values,
                           Smp::UInt64 startIndex = 0) override;

  Smp::Bool AddChild(Smp::IObject *child,
                     const Smp::IObject *collection) override;
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::IObject *collection) override;
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
