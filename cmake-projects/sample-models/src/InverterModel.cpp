#include "InverterModel.hpp"
#include <Smp/IPublication.h>
#include <stdexcept>

namespace sample {

const Smp::Uuid InverterModel::_uuid = {0x12345678, 0x1234, 0x5678, 0x1234,
                                        0x567812345678};

InverterModel::InverterModel(Smp::String8 name, Smp::String8 description,
                             Smp::IComposite *parent)
    : core::Object(name, description, parent),
      _state(Smp::ComponentStateKind::CSK_Created), _input(false),
      _output(true) {}

Smp::ComponentStateKind InverterModel::GetState() const { return _state; }

void InverterModel::Publish(Smp::IPublication *receiver) {
  if (receiver) {
    receiver->PublishField("Input", "Boolean Input", &_input,
                           Smp::ViewKind::VK_All, true, true, false);
    receiver->PublishField("Output", "Boolean Output", &_output,
                           Smp::ViewKind::VK_All, true, false, true);
  }
  _state = Smp::ComponentStateKind::CSK_Publishing;
}

void InverterModel::Configure(Smp::Services::ILogger *logger,
                              Smp::Services::ILinkRegistry *linkRegistry) {
  _state = Smp::ComponentStateKind::CSK_Configured;
}

void InverterModel::Connect(Smp::ISimulator *simulator) {
  _state = Smp::ComponentStateKind::CSK_Connected;
}

void InverterModel::Disconnect() {
  _state = Smp::ComponentStateKind::CSK_Created;
}

const Smp::Uuid &InverterModel::GetUuid() const { return _uuid; }

Smp::IField *InverterModel::GetField(Smp::String8 fullName) const {
  return nullptr;
}
const Smp::FieldCollection *InverterModel::GetFields() const { return nullptr; }

Smp::AnySimple InverterModel::GetSimpleValue(Smp::String8 fullName) const {
  return Smp::AnySimple();
}
void InverterModel::SetSimpleValue(Smp::String8 fullName,
                                   Smp::AnySimple value) {}
void InverterModel::GetSimpleArrayValue(Smp::String8 fullName,
                                        Smp::UInt64 length,
                                        Smp::AnySimple *values,
                                        Smp::UInt64 startIndex) const {}
void InverterModel::SetSimpleArrayValue(Smp::String8 fullName,
                                        Smp::UInt64 length,
                                        Smp::AnySimpleArray values,
                                        Smp::UInt64 startIndex) {}

Smp::Bool InverterModel::AddChild(Smp::IObject *child,
                                  const Smp::ICollectionBase *collection) {
  return false;
}
Smp::Bool InverterModel::RemoveChild(Smp::IObject *child,
                                     const Smp::ICollectionBase *collection) {
  return false;
}
Smp::IObject *InverterModel::IsChildInCollection(
    Smp::String8 child, const Smp::ICollectionBase *collection) const {
  return nullptr;
}

void InverterModel::Execute() { _output = !_input; }

} // namespace sample
