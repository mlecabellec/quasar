#include "DelayedModels.hpp"
#include <Smp/IPublication.h>
#include <Smp/IField.h>
#include <Smp/ISimpleArrayField.h>
#include <cstring>
#include <string>

namespace sample {

const Smp::Uuid DelayedSimpleModel::_uuid = {0xAAA11111, 0x1111, 0x1111, 0x1111, 0x111111111111};

DelayedSimpleModel::DelayedSimpleModel(Smp::String8 name, Smp::String8 description, Smp::IObject* parent)
    : SmpModel(name, description, parent),
      _input{}, _mem1{}, _mem2{}, _output{},
      _executeEntryPoint(this) {}

void DelayedSimpleModel::Publish(Smp::IPublication* receiver) {
    SmpModel::Publish(receiver);
    if (!receiver) return;

    // Publish Inputs
    _fields.Add(receiver->PublishField("Input_Bool", "", &_input.b, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_Char8", "", &_input.c8, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_Int8", "", &_input.i8, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_Int16", "", &_input.i16, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_Int32", "", &_input.i32, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_Int64", "", &_input.i64, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_UInt8", "", &_input.u8, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_UInt16", "", &_input.u16, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_UInt32", "", &_input.u32, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_UInt64", "", &_input.u64, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_Float32", "", &_input.f32, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_Float64", "", &_input.f64, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_Duration", "", &_input.dur, Smp::ViewKind::VK_All, true, true, false));
    _fields.Add(receiver->PublishField("Input_DateTime", "", &_input.dt, Smp::ViewKind::VK_All, true, true, false));

    // Publish Outputs
    _fields.Add(receiver->PublishField("Output_Bool", "", &_output.b, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_Char8", "", &_output.c8, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_Int8", "", &_output.i8, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_Int16", "", &_output.i16, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_Int32", "", &_output.i32, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_Int64", "", &_output.i64, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_UInt8", "", &_output.u8, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_UInt16", "", &_output.u16, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_UInt32", "", &_output.u32, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_UInt64", "", &_output.u64, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_Float32", "", &_output.f32, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_Float64", "", &_output.f64, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_Duration", "", &_output.dur, Smp::ViewKind::VK_All, true, false, true));
    _fields.Add(receiver->PublishField("Output_DateTime", "", &_output.dt, Smp::ViewKind::VK_All, true, false, true));
}

Smp::IObject* DelayedSimpleModel::GetChild(Smp::String8 name) const {
    if (std::string(name) == "Execute") {
        return const_cast<ExecuteEntryPoint*>(&_executeEntryPoint);
    }
    return SmpModel::GetChild(name);
}

const Smp::Uuid& DelayedSimpleModel::GetUuid() const { return _uuid; }

void DelayedSimpleModel::Execute() {
    _output = _mem2;
    _mem2 = _mem1;
    _mem1 = _input;
}

const Smp::Uuid DelayedArrayModel::_uuid = {0xAAA22222, 0x2222, 0x2222, 0x2222, 0x222222222222};

DelayedArrayModel::DelayedArrayModel(Smp::String8 name, Smp::String8 description, Smp::IObject* parent)
    : SmpModel(name, description, parent),
      _input{}, _mem1{}, _mem2{}, _output{},
      _executeEntryPoint(this) {}

void DelayedArrayModel::Publish(Smp::IPublication* receiver) {
    SmpModel::Publish(receiver);
    if (!receiver) return;

    _fields.Add(dynamic_cast<Smp::IField*>(receiver->PublishArray("InputArray", "Input Array of Int8", ARRAY_SIZE, _input, Smp::PrimitiveTypeKind::PTK_Int8, Smp::ViewKind::VK_All, true, true, false)));
    _fields.Add(dynamic_cast<Smp::IField*>(receiver->PublishArray("OutputArray", "Output Array of Int8", ARRAY_SIZE, _output, Smp::PrimitiveTypeKind::PTK_Int8, Smp::ViewKind::VK_All, true, false, true)));
}

Smp::IObject* DelayedArrayModel::GetChild(Smp::String8 name) const {
    if (std::string(name) == "Execute") {
        return const_cast<ExecuteEntryPoint*>(&_executeEntryPoint);
    }
    return SmpModel::GetChild(name);
}

const Smp::Uuid& DelayedArrayModel::GetUuid() const { return _uuid; }

void DelayedArrayModel::Execute() {
    std::memcpy(_output, _mem2, sizeof(_output));
    std::memcpy(_mem2, _mem1, sizeof(_mem2));
    std::memcpy(_mem1, _input, sizeof(_mem1));
}

} // namespace sample
