#include "common/SmpModel.hpp"
#include <Smp/IPublication.h>

namespace sample {

SmpModel::SmpModel(Smp::String8 name, Smp::String8 description, Smp::IObject* parent)
    : _name(name ? name : ""),
      _description(description ? description : ""),
      _parent(parent),
      _simulator(nullptr),
      _state(Smp::ComponentStateKind::CSK_Created) {}

Smp::String8 SmpModel::GetName() const { return _name.c_str(); }
Smp::String8 SmpModel::GetDescription() const { return _description.c_str(); }
Smp::IObject* SmpModel::GetParent() const { return _parent; }

Smp::IObject* SmpModel::GetChild(Smp::String8 name) const {
    if (!name) return nullptr;
    if (auto* field = _fields.at(name)) return field;
    if (auto* container = _containers.at(name)) return container;
    return nullptr;
}

const Smp::ContainerCollection* SmpModel::GetContainers() const { return &_containers; }
Smp::IContainer* SmpModel::GetContainer(Smp::String8 name) const { return _containers.at(name); }

Smp::ComponentStateKind SmpModel::GetState() const { return _state; }

void SmpModel::Publish(Smp::IPublication* receiver) {
    // In a full implementation, we would check if we are in Created state.
    _state = Smp::ComponentStateKind::CSK_Publishing;
}

void SmpModel::Configure(Smp::Services::ILogger* logger, Smp::Services::ILinkRegistry* linkRegistry) {
    _state = Smp::ComponentStateKind::CSK_Configured;
}

void SmpModel::Connect(Smp::ISimulator* simulator) {
    _simulator = simulator;
    _state = Smp::ComponentStateKind::CSK_Connected;
}

void SmpModel::Disconnect() {
    _state = Smp::ComponentStateKind::CSK_Disconnected;
}

Smp::IField* SmpModel::GetField(Smp::String8 fullName) const { return _fields.at(fullName); }
const Smp::FieldCollection* SmpModel::GetFields() const { return &_fields; }

Smp::AnySimple SmpModel::GetSimpleValue(Smp::String8 fullName) const { 
    return Smp::AnySimple(); 
}

void SmpModel::SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) {}

void SmpModel::GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length, Smp::AnySimple* values, Smp::UInt64 startIndex) const {}
void SmpModel::SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length, Smp::AnySimpleArray values, Smp::UInt64 startIndex) {}

Smp::Bool SmpModel::AddChild(Smp::IObject* child, const Smp::ICollectionBase* collection) { return false; }
Smp::Bool SmpModel::RemoveChild(Smp::IObject* child, const Smp::ICollectionBase* collection) { return false; }
Smp::IObject* SmpModel::IsChildInCollection(Smp::String8 child, const Smp::ICollectionBase* collection) const { return nullptr; }

} // namespace sample
