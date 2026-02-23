#pragma once

#include <Smp/IEntryPoint.h>
#include <Smp/Services/IEventManager.h>
#include <core/Object.hpp>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace utils {

/**
 * @brief Implementation of the SMP Event Manager Service.
 * @details This class provides mechanisms for event subscription, unsubscription, and emission.
 * Contributes to [FE-0070.3.1].
 */
class EventManager : public core::Object,
                     public virtual Smp::Services::IEventManager {
public:
  /**
   * @brief Default constructor.
   */
  EventManager();

  /**
   * @brief Virtual destructor.
   */
  virtual ~EventManager() noexcept override = default;

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

  Smp::IObject *GetChild(Smp::String8 name) const override;

  // IEventManager methods

  /**
   * @brief Query the event ID for a given event name.
   * @param eventName Name of the event.
   * @return EventId associated with the name.
   */
  Smp::Services::EventId QueryEventId(Smp::String8 eventName) override;

  /**
   * @brief Subscribe an entry point to an event.
   * @param event The event ID.
   * @param entryPoint The entry point to subscribe.
   */
  void Subscribe(Smp::Services::EventId event,
                 const Smp::IEntryPoint *entryPoint) override;

  /**
   * @brief Unsubscribe an entry point from an event.
   * @param event The event ID.
   * @param entryPoint The entry point to unsubscribe.
   */
  void Unsubscribe(Smp::Services::EventId event,
                   const Smp::IEntryPoint *entryPoint) override;

  /**
   * @brief Emit an event.
   * @param event The event ID.
   * @param synchronous Whether the emission is synchronous.
   */
  void Emit(Smp::Services::EventId event,
            Smp::Bool synchronous = true) override;

private:
  /** @brief Next available event ID for dynamic creation. */
  Smp::Services::EventId _nextEventId = 20;
  
  /** @brief Mapping from event name to event ID. */
  std::map<std::string, Smp::Services::EventId> _eventIds;
  
  /** @brief Mapping from event ID to event name. */
  std::map<Smp::Services::EventId, std::string> _eventNames;
  
  /** @brief Subscription list for each event ID. */
  std::map<Smp::Services::EventId, std::vector<const Smp::IEntryPoint *>> _subscriptions;

  /** @brief Mutex for thread-safe access. */
  mutable std::timed_mutex _mutex;

  /**
   * @brief Register standard SMP predefined events.
   */
  void RegisterPredefinedEvents();
};

} // namespace utils
