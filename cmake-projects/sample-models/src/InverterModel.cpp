#include "InverterModel.hpp"
#include <Smp/IPublication.h>
#include <string>

namespace sample {

const Smp::Uuid InverterModel::_uuid = {0x12345678, 0x1234, 0x5678, 0x1234,
                                        0x567812345678};

InverterModel::InverterModel(Smp::String8 name, Smp::String8 description,
                             Smp::IComposite *parent)
    : SmpModel(name, description, parent),
      _input(false),
      _output(true),
      _executeEntryPoint(this) {}

void InverterModel::Publish(Smp::IPublication *receiver) {
  SmpModel::Publish(receiver);
  if (receiver) {
    _fields.Add(receiver->PublishField("Input", "Boolean Input", &_input,
                           Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Output", "Boolean Output", &_output,
                           Smp::ViewKind::VK_All, true, false, true));
  }
}

const Smp::Uuid &InverterModel::GetUuid() const { return _uuid; }

Smp::AnySimple InverterModel::GetSimpleValue(Smp::String8 fullName) const {
  if (std::string(fullName) == "Input") {
    return Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, _input);
  } else if (std::string(fullName) == "Output") {
    return Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, _output);
  }
  return SmpModel::GetSimpleValue(fullName);
}

void InverterModel::SetSimpleValue(Smp::String8 fullName,
                                   Smp::AnySimple value) {
  if (std::string(fullName) == "Input") {
    _input = (Smp::Bool)value;
  } else {
    SmpModel::SetSimpleValue(fullName, value);
  }
}

Smp::IObject *InverterModel::GetChild(Smp::String8 name) const {
  if (std::string(name) == "Execute") {
    return const_cast<ExecuteEntryPoint *>(&_executeEntryPoint);
  }
  return SmpModel::GetChild(name);
}

void InverterModel::Execute() { _output = !_input; }

} // namespace sample
