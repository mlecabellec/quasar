#pragma once

#include <Smp/IEntryPoint.h>
#include <Smp/Services/IEventManager.h>
#include <core/Object.hpp>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace utils {

class EventManager : public core::Object,
                     public virtual Smp::Services::IEventManager {
public:
  EventManager();
  virtual ~EventManager() noexcept = default;

  // IComponent methods
  Smp::ComponentStateKind GetState() const override;
  void Publish(Smp::IPublication *receiver) override;
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override;
  void Connect(Smp::ISimulator *simulator) override;
  void Disconnect() override;
  const Smp::Uuid &GetUuid() const override;

  Smp::IField *GetField(Smp::String8 fullName) const override;
  const Smp::FieldCollection *GetFields() const override;
  Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;
  void SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) override;
  void GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimple *values,
                           Smp::UInt64 startIndex = 0) const override;
  void SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimpleArray values,
                           Smp::UInt64 startIndex = 0) override;
  Smp::Bool AddChild(Smp::IObject *child,
                     const Smp::ICollectionBase *collection) override;
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::ICollectionBase *collection) override;
  Smp::IObject *
  IsChildInCollection(Smp::String8 child,
                      const Smp::ICollectionBase *collection) const override;

  // IEventManager methods

  Smp::Services::EventId QueryEventId(Smp::String8 eventName) override;

  void Subscribe(Smp::Services::EventId event,
                 const Smp::IEntryPoint *entryPoint) override;
  void Unsubscribe(Smp::Services::EventId event,
                   const Smp::IEntryPoint *entryPoint) override;
  void Emit(Smp::Services::EventId event,
            Smp::Bool synchronous = true) override;

private:
  Smp::Services::EventId _nextEventId = 20; // Start after pre-defined events
  std::map<std::string, Smp::Services::EventId> _eventIds;
  std::map<Smp::Services::EventId, std::string> _eventNames;
  std::map<Smp::Services::EventId, std::vector<const Smp::IEntryPoint *>>
      _subscriptions;

  mutable std::mutex _mutex;

  void RegisterPredefinedEvents();
};

} // namespace utils
