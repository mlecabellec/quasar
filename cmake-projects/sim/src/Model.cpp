#include "sim/Model.hpp"
#include <core/StandardExceptions.hpp>

namespace sim {

Model::Model(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
             Smp::ISimulator *simulator)
    : core::Object(name, description, parent), _simulator(simulator) {
  _state = Smp::ComponentStateKind::CSK_Created;
}

// IComponent methods
Smp::ComponentStateKind Model::GetState() const { return _state; }

void Model::Publish(Smp::IPublication *receiver) {
  /// Fulfills [FE-0060.3.4] (IComponent::Publish).
  // [FE-0050.6.1] Transitions component state from CSK_Created to CSK_Publishing.
  if (_state != Smp::ComponentStateKind::CSK_Created) {
    throw core::InvalidComponentState(_state,
                                      Smp::ComponentStateKind::CSK_Created);
  }
  _state = Smp::ComponentStateKind::CSK_Publishing;
}

void Model::Configure(Smp::Services::ILogger *logger,
                      Smp::Services::ILinkRegistry *linkRegistry) {
  /// Fulfills [FE-0060.3.5] (IComponent::Configure).
  // [FE-0050.6.1] Transitions component state from CSK_Publishing to CSK_Configured.
  if (_state != Smp::ComponentStateKind::CSK_Publishing) {
    throw core::InvalidComponentState(_state,
                                      Smp::ComponentStateKind::CSK_Publishing);
  }
  _state = Smp::ComponentStateKind::CSK_Configured;
}

void Model::Connect(Smp::ISimulator *simulator) {
  /// Fulfills [FE-0060.3.6] (IComponent::Connect).
  // [FE-0050.6.1] Transitions component state from CSK_Configured to CSK_Connected.
  if (_state != Smp::ComponentStateKind::CSK_Configured)
    throw core::InvalidComponentState(_state,
                                      Smp::ComponentStateKind::CSK_Configured);

  _simulator = simulator;

  _state = Smp::ComponentStateKind::CSK_Connected;
}

void Model::Disconnect() {
  // [FE-0050.6.1] Transitions component state, typically back to CSK_Created or CSK_Configured.
  // Check state? The standard doesn't explicitly restrict Disconnect from all
  // states, but usually from Connected. "Ask the component to disconnect...".
  _state =
      Smp::ComponentStateKind::CSK_Created; // Reset to created? Or Configured?
  // Spec doesn't detail the target state of Disconnect clearly in generic doc,
  // usually goes back to Created or Configured. Reconnect() says: "reconnect
  // the component hierarchy... starting at given root". Disconnect usually
  // leads to potential destruction or reconnection. Let's assume it goes to
  // Configured or Created. For now, irrelevant for basic sim.
}

const Smp::ContainerCollection *Model::GetContainers() const {
  return &_containers;
}

Smp::IContainer *Model::GetContainer(Smp::String8 name) const {
  return _containers.at(name);
}

Smp::IField *Model::GetField(Smp::String8 fullName) const {
  return _fields.at(fullName);
}

const Smp::FieldCollection *Model::GetFields() const { return &_fields; }

const Smp::Uuid &Model::GetUuid() const {
  static Smp::Uuid validUuid; // TODO: Generate or allow setting via constructor
  return validUuid;
}

Smp::AnySimple Model::GetSimpleValue(Smp::String8 fullName) const {
  // Potentially interacts with primitive types via Smp::AnySimple, supporting
  // FE-0050.1.6 and FE-0050.1.7. Currently stubbed.
  throw core::InvalidFieldName(fullName);
}

void Model::SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) {
  // Potentially interacts with primitive types via Smp::AnySimple, supporting
  // FE-0050.1.6 and FE-0050.1.7. Currently stubbed.
  throw core::InvalidFieldName(fullName);
}

void Model::GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                Smp::AnySimple *values,
                                Smp::UInt64 startIndex) const {
  // Potentially interacts with primitive types via Smp::AnySimpleArray, supporting
  // FE-0050.1.8. Currently stubbed.
  throw core::InvalidFieldName(fullName);
}

void Model::SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                Smp::AnySimpleArray values,
                                Smp::UInt64 startIndex) {
  // Potentially interacts with primitive types via Smp::AnySimpleArray, supporting
  // FE-0050.1.8. Currently stubbed.
  throw core::InvalidFieldName(fullName);
}

Smp::Bool Model::AddChild(Smp::IObject *child,
                          const Smp::ICollectionBase *collection) {
  return false;
}

Smp::Bool Model::RemoveChild(Smp::IObject *child,
                             const Smp::ICollectionBase *collection) {
  return false;
}

Smp::IObject *
Model::IsChildInCollection(Smp::String8 child,
                           const Smp::ICollectionBase *collection) const {
  if (!collection)
    return nullptr;
  // Check if collection is one of ours
  if (collection == static_cast<const Smp::ICollectionBase *>(&_containers))
    return _containers.at(child);
  if (collection == static_cast<const Smp::ICollectionBase *>(&_fields))
    return _fields.at(child);
  return nullptr;
}

Smp::IObject *Model::GetChild(Smp::String8 name) const {
  if (auto *c = _containers.at(name))
    return c;
  if (auto *f = _fields.at(name))
    return f;
  return nullptr;
}

// IObject methods
Smp::String8 Model::GetName() const { return core::Object::GetName(); }
Smp::String8 Model::GetDescription() const {
  return core::Object::GetDescription();
}
Smp::IObject *Model::GetParent() const { return core::Object::GetParent(); }

} // namespace sim
