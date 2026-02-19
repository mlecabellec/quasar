#pragma once

#include <Smp/DuplicateName.h>
#include <Smp/InvalidComponentState.h>
#include <Smp/InvalidFieldName.h>
#include <Smp/InvalidObjectType.h>
#include <Smp/InvalidSimulatorState.h>
#include <Smp/Services/EntryPointAlreadySubscribed.h>
#include <Smp/Services/EntryPointNotSubscribed.h>
#include <Smp/Services/InvalidEventId.h>
#include <Smp/Services/InvalidEventName.h>
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
  Smp::String8 GetName() const override { return core::Exception::GetName(); }
  Smp::String8 GetDescription() const override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const override {
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

  Smp::String8 GetName() const override { return core::Exception::GetName(); }
  Smp::String8 GetDescription() const override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const override {
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

  Smp::String8 GetName() const override { return core::Exception::GetName(); }
  Smp::String8 GetDescription() const override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const override {
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

  Smp::String8 GetName() const override { return core::Exception::GetName(); }
  Smp::String8 GetDescription() const override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const override {
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

  Smp::String8 GetName() const override { return core::Exception::GetName(); }
  Smp::String8 GetDescription() const override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const override {
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

  Smp::String8 GetName() const override { return core::Exception::GetName(); }
  Smp::String8 GetDescription() const override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const override {
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

  Smp::String8 GetName() const override { return core::Exception::GetName(); }
  Smp::String8 GetDescription() const override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const override {
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

  Smp::String8 GetName() const override { return core::Exception::GetName(); }
  Smp::String8 GetDescription() const override {
    return core::Exception::GetDescription();
  }
  Smp::String8 GetMessage() const override {
    return core::Exception::GetMessage();
  }

private:
  const Smp::IEntryPoint *_entryPoint;
  std::string _eventName;
};

} // namespace core
