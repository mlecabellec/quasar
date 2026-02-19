#pragma once

#ifndef CORE_STANDARDEXCEPTIONS_HPP_
#define CORE_STANDARDEXCEPTIONS_HPP_

#include <Smp/CannotDelete.h>
#include <Smp/ContainerFull.h>
#include <Smp/DuplicateName.h>
#include <Smp/DuplicateUuid.h>
#include <Smp/InvalidComponentState.h>
#include <Smp/InvalidFieldName.h>
#include <Smp/InvalidObjectName.h>
#include <Smp/InvalidObjectType.h>
#include <Smp/InvalidSimulatorState.h>
#include <Smp/NotContained.h>
#include <Smp/Services/EntryPointAlreadySubscribed.h>
#include <Smp/Services/EntryPointNotSubscribed.h>
#include <Smp/Services/InvalidCycleTime.h>
#include <Smp/Services/InvalidEventId.h>
#include <Smp/Services/InvalidEventName.h>
#include <Smp/Services/InvalidEventTime.h>
#include <core/Exception.hpp>

namespace core {

class InvalidSimulatorState : public core::Exception,
                              public Smp::InvalidSimulatorState {
public:
  InvalidSimulatorState(Smp::SimulatorStateKind state)
      : core::Exception("InvalidSimulatorState", "Invalid Simulator State"),
        _state(state) {
    _message = "Invalid Simulator State";
  }

  Smp::SimulatorStateKind GetInvalidState() const noexcept override {
    return _state;
  }

  // Ambiguity resolution
  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }

private:
  Smp::SimulatorStateKind _state;
};

class EntryPointNotSubscribed : public core::Exception,
                                public Smp::Services::EntryPointNotSubscribed {
public:
  EntryPointNotSubscribed(const Smp::IEntryPoint *entryPoint,
                          Smp::String8 eventName)
      : core::Exception("EntryPointNotSubscribed",
                        "Entry Point Not Subscribed"),
        _entryPoint(entryPoint), _eventName(eventName ? eventName : "") {
    _message = "Entry point not subscribed to event: " + _eventName;
  }

  const Smp::IEntryPoint *GetEntryPoint() const noexcept override {
    return _entryPoint;
  }

  Smp::String8 GetEventName() const noexcept override {
    return _eventName.c_str();
  }

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }

private:
  const Smp::IEntryPoint *_entryPoint;
  std::string _eventName;
};

class InvalidComponentState : public core::Exception,
                              public Smp::InvalidComponentState {
public:
  InvalidComponentState(Smp::ComponentStateKind state,
                        Smp::ComponentStateKind expected)
      : core::Exception("InvalidComponentState", "Invalid Component State"),
        _state(state), _expected(expected) {
    _message = "Invalid Component State";
  }

  Smp::ComponentStateKind GetInvalidState() const noexcept override {
    return _state;
  }
  Smp::ComponentStateKind GetExpectedState() const noexcept override {
    return _expected;
  }

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }

private:
  Smp::ComponentStateKind _state;
  Smp::ComponentStateKind _expected;
};

class InvalidFieldName : public core::Exception, public Smp::InvalidFieldName {
public:
  InvalidFieldName(Smp::String8 name)
      : core::Exception("InvalidFieldName", "Invalid Field Name"),
        _fieldName(name ? name : "") {
    _message = "Invalid Field Name: " + _fieldName;
  }

  Smp::String8 GetFieldName() const noexcept override {
    return _fieldName.c_str();
  }

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }

private:
  std::string _fieldName;
};

class DuplicateName : public core::Exception, public Smp::DuplicateName {
public:
  DuplicateName(Smp::String8 name)
      : core::Exception("DuplicateName", "Duplicate Name"),
        _duplicateName(name ? name : "") {
    _message = "Duplicate Name: " + _duplicateName;
  }

  Smp::String8 GetDuplicateName() const noexcept override {
    return _duplicateName.c_str();
  }

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }

private:
  std::string _duplicateName;
};

class InvalidEventId : public core::Exception,
                       public Smp::Services::InvalidEventId {
public:
  InvalidEventId(Smp::Services::EventId id)
      : core::Exception("InvalidEventId", "Invalid Event Id"), _id(id) {
    _message = "Invalid Event Id: " + std::to_string(_id);
  }

  Smp::Services::EventId GetInvalidEventId() const noexcept override {
    return _id;
  }

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }

private:
  Smp::Services::EventId _id;
};

class InvalidEventName : public core::Exception,
                         public Smp::Services::InvalidEventName {
public:
  InvalidEventName(Smp::String8 name)
      : core::Exception("InvalidEventName", "Invalid Event Name") {
    _message = "Invalid Event Name: " +
               (name ? std::string(name) : std::string("null"));
  }

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }
};

class EntryPointAlreadySubscribed
    : public core::Exception,
      public Smp::Services::EntryPointAlreadySubscribed {
public:
  EntryPointAlreadySubscribed(const Smp::IEntryPoint *entryPoint,
                              Smp::String8 eventName)
      : core::Exception("EntryPointAlreadySubscribed",
                        "Entry Point Already Subscribed"),
        _entryPoint(entryPoint), _eventName(eventName ? eventName : "") {
    _message = "Entry point already subscribed to event: " + _eventName;
  }

  const Smp::IEntryPoint *GetEntryPoint() const noexcept override {
    return _entryPoint;
  }
  Smp::String8 GetEventName() const noexcept override {
    return _eventName.c_str();
  }

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }

private:
  const Smp::IEntryPoint *_entryPoint;
  std::string _eventName;
};

class InvalidEventTime : public core::Exception,
                         public Smp::Services::InvalidEventTime {
public:
  InvalidEventTime(Smp::String8 message)
      : core::Exception("InvalidEventTime", message ? message : "") {
    _message = message ? message : "";
  }

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }
};

class InvalidCycleTime : public core::Exception,
                         public Smp::Services::InvalidCycleTime {
public:
  InvalidCycleTime(Smp::String8 message)
      : core::Exception("InvalidCycleTime", message ? message : "") {
    _message = message ? message : "";
  }

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }
};

class DuplicateUuid : public core::Exception, public Smp::DuplicateUuid {
public:
  DuplicateUuid(Smp::String8 oldName, Smp::String8 newName)
      : core::Exception("DuplicateUuid", "Duplicate UUID"),
        _oldName(oldName ? oldName : ""), _newName(newName ? newName : "") {}

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }
  Smp::String8 GetOldName() const noexcept override { return _oldName.c_str(); }
  Smp::String8 GetNewName() const noexcept override { return _newName.c_str(); }

  std::string _oldName;
  std::string _newName;
};

class InvalidObjectName : public core::Exception,
                          public Smp::InvalidObjectName {
public:
  InvalidObjectName(Smp::String8 name, Smp::String8 message)
      : core::Exception("InvalidObjectName", message ? message : ""),
        _invalidName(name ? name : ""), _message(message ? message : "") {}

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }
  Smp::String8 GetInvalidName() const noexcept override {
    return _invalidName.c_str();
  }

private:
  std::string _invalidName;
  std::string _message;
};

class ContainerFull : public core::Exception, public Smp::ContainerFull {
public:
  ContainerFull(Smp::String8 containerName, Smp::Int64 containerSize)
      : core::Exception("ContainerFull", "Container is full"),
        _containerName(containerName ? containerName : ""),
        _containerSize(containerSize) {}

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }
  Smp::String8 GetContainerName() const noexcept override {
    return _containerName.c_str();
  }
  Smp::Int64 GetContainerSize() const noexcept override {
    return _containerSize;
  }

private:
  std::string _containerName;
  Smp::Int64 _containerSize;
};

class NotContained : public core::Exception, public Smp::NotContained {
public:
  NotContained(Smp::String8 containerName, const Smp::IComponent *component)
      : core::Exception("NotContained", "Component not contained"),
        _containerName(containerName ? containerName : ""),
        _component(component) {}

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }
  Smp::String8 GetContainerName() const noexcept override {
    return _containerName.c_str();
  }
  const Smp::IComponent *GetComponent() const noexcept override {
    return _component;
  }

private:
  std::string _containerName;
  const Smp::IComponent *_component;
};

class CannotDelete : public core::Exception, public Smp::CannotDelete {
public:
  CannotDelete(Smp::String8 containerName, const Smp::IComponent *component,
               Smp::Int64 lowerLimit)
      : core::Exception("CannotDelete", "Cannot delete below minimum"),
        _containerName(containerName ? containerName : ""),
        _component(component), _lowerLimit(lowerLimit) {}

  Smp::String8 GetName() const noexcept override {
    return core::Exception::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override {
    return core::Exception::GetMessage();
  }
  Smp::String8 GetContainerName() const noexcept override {
    return _containerName.c_str();
  }
  const Smp::IComponent *GetComponent() const noexcept override {
    return _component;
  }
  Smp::Int64 GetLowerLimit() const noexcept override { return _lowerLimit; }

private:
  std::string _containerName;
  const Smp::IComponent *_component;
  Smp::Int64 _lowerLimit;
};

} // namespace core

#endif // CORE_STANDARDEXCEPTIONS_HPP_
