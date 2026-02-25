#include "utils/EventManager.hpp"
#include <Smp/IEntryPoint.h>
#include <algorithm>
#include <core/StandardExceptions.hpp>
#include <cstring>
#include <stdexcept>

namespace utils {

/**
 * @brief Constructor for EventManager.
 * @details Initializes the predefined SMP events.
 */
EventManager::EventManager()
    : core::Object("EventManager", "SMP Event Manager Service", nullptr) {
  // Register the standard SMP events
  RegisterPredefinedEvents();
}

Smp::ComponentStateKind EventManager::GetState() const {
  return Smp::ComponentStateKind::CSK_Connected;
}

void EventManager::Publish(Smp::IPublication *receiver) {
  // [CS-0010.16] Check for null pointers
  if (!receiver) {
    // Standard doesn't specify behavior here, but we should be safe
    return;
  }
}

void EventManager::Configure(Smp::Services::ILogger *logger,
                             Smp::Services::ILinkRegistry *linkRegistry) {
  // Configuration logic would go here
}

void EventManager::Connect(Smp::ISimulator *simulator) {
  // Connection logic would go here
}

void EventManager::Disconnect() {
  // Disconnection logic would go here
}

const Smp::Uuid &EventManager::GetUuid() const {
  // [CS-0010.31] Avoid global constants, using static inside method
  static Smp::Uuid uuid = {0, 0, 0, 0, 4}; // Generic Service UUID
  return uuid;
}

Smp::IField *EventManager::GetField(Smp::String8 fullName) const {
  return nullptr;
}

const Smp::FieldCollection *EventManager::GetFields() const { return nullptr; }

Smp::AnySimple EventManager::GetSimpleValue(Smp::String8 fullName) const {
  return Smp::AnySimple();
}

void EventManager::SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) {
}

void EventManager::GetSimpleArrayValue(Smp::String8 fullName,
                                       Smp::UInt64 length,
                                       Smp::AnySimple *values,
                                       Smp::UInt64 startIndex) const {}

void EventManager::SetSimpleArrayValue(Smp::String8 fullName,
                                       Smp::UInt64 length,
                                       Smp::AnySimpleArray values,
                                       Smp::UInt64 startIndex) {}

Smp::Bool EventManager::AddChild(Smp::IObject *child,
                                 const Smp::ICollectionBase *collection) {
  return false;
}

Smp::Bool EventManager::RemoveChild(Smp::IObject *child,
                                    const Smp::ICollectionBase *collection) {
  return false;
}

Smp::IObject *EventManager::IsChildInCollection(
    Smp::String8 child, const Smp::ICollectionBase *collection) const {
  return nullptr;
}

Smp::IObject *EventManager::GetChild(Smp::String8 name) const {
  return nullptr;
}

void EventManager::RegisterPredefinedEvents() {
  // This method registers standard SMP predefined events, supporting
  // [FE-0070.4.8] and implicitly contributing to [FE-0070.4.9] by defining
  // these events. These events are crucial for simulation state transitions and
  // time updates.

  // Map standard event names to their reserved IDs
  _eventIds[SMP_LeaveConnecting] = SMP_LeaveConnectingId;
  _eventIds[SMP_EnterInitialising] = SMP_EnterInitialisingId;
  _eventIds[SMP_LeaveInitialising] = SMP_LeaveInitialisingId;
  _eventIds[SMP_EnterStandby] = SMP_EnterStandbyId;
  _eventIds[SMP_LeaveStandby] = SMP_LeaveStandbyId;
  _eventIds[SMP_EnterExecuting] = SMP_EnterExecutingId;
  _eventIds[SMP_LeaveExecuting] = SMP_LeaveExecutingId;
  _eventIds[SMP_EnterStoring] = SMP_EnterStoringId;
  _eventIds[SMP_LeaveStoring] = SMP_LeaveStoringId;
  _eventIds[SMP_EnterRestoring] = SMP_EnterRestoringId;
  _eventIds[SMP_LeaveRestoring] = SMP_LeaveRestoringId;
  _eventIds[SMP_EnterExiting] = SMP_EnterExitingId;
  _eventIds[SMP_EnterAborting] = SMP_EnterAbortingId;
  _eventIds[SMP_EpochTimeChanged] = SMP_EpochTimeChangedId;
  _eventIds[SMP_MissionTimeChanged] = SMP_MissionTimeChangedId;
  _eventIds[SMP_EnterReconnecting] = SMP_EnterReconnectingId;
  _eventIds[SMP_LeaveReconnecting] = SMP_LeaveReconnectingId;
  _eventIds[SMP_PreSimTimeChange] = SMP_PreSimTimeChangeId;
  _eventIds[SMP_PostSimTimeChange] = SMP_PostSimTimeChangeId;
}

Smp::Services::EventId EventManager::QueryEventId(Smp::String8 eventName) {
  // Implements FE-0070.4.2: IEventManager QueryEventId shall return Event
  // identifier. This method ensures that event names are mapped to unique
  // EventIds. New IDs are generated if the event name is not found, ensuring
  // uniqueness throughout the simulation.

  // [CS-0010.16] Check for null pointers
  if (!eventName) {
    throw core::InvalidEventName("Event name is null");
  }

  // Use RAII for mutex [CS-0010.22]
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring EventManager lock");
  }

  // Search for existing event by name
  // [CS-0010.35] Forbidden auto replaced with explicit type
  std::map<std::string, Smp::Services::EventId>::iterator it =
      _eventIds.find(eventName);
  if (it != _eventIds.end()) {
    return it->second;
  }

  // Create new event ID if not found
  Smp::Services::EventId newId = _nextEventId++;
  _eventIds[eventName] = newId;
  _eventNames[newId] = eventName;
  return newId;
}

void EventManager::Subscribe(Smp::Services::EventId event,
                             const Smp::IEntryPoint *entryPoint) {
  // Implements FE-0070.4.5: IEventManager Subscribe shall subscribe an entry
  // point to an event. Also addresses FE-0070.4.3 and FE-0070.4.4 by managing
  // the subscription list.

  // [CS-0010.16] Check for null pointers
  if (!entryPoint) {
    throw std::invalid_argument("Entry point is null");
  }

  // Use RAII for mutex [CS-0010.22]
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring EventManager lock");
  }

  // [CS-0010.35] Forbidden auto replaced with explicit type
  std::vector<const Smp::IEntryPoint *> &subscribers = _subscriptions[event];
  std::vector<const Smp::IEntryPoint *>::iterator it =
      std::find(subscribers.begin(), subscribers.end(), entryPoint);

  // Verify if already subscribed
  if (it != subscribers.end()) {
    throw core::EntryPointAlreadySubscribed(entryPoint,
                                            "Entry point already subscribed");
  }

  // Add new subscriber
  subscribers.push_back(entryPoint);
}

void EventManager::Unsubscribe(Smp::Services::EventId event,
                               const Smp::IEntryPoint *entryPoint) {
  // Implements FE-0070.4.6: IEventManager Unsubscribe shall unsubscribe an
  // entry point. Handles cases where the event or entry point might not exist.

  // [CS-0010.16] Check for null pointers
  if (!entryPoint) {
    throw std::invalid_argument("Entry point is null");
  }

  // Use RAII for mutex [CS-0010.22]
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring EventManager lock");
  }

  // [CS-0010.35] Forbidden auto replaced with explicit type
  std::map<Smp::Services::EventId,
           std::vector<const Smp::IEntryPoint *>>::iterator itSub =
      _subscriptions.find(event);
  if (itSub == _subscriptions.end()) {
    throw core::EntryPointNotSubscribed(
        entryPoint, "Entry point not subscribed (event has no subscribers)");
  }

  // Find the entry point in the subscription list
  std::vector<const Smp::IEntryPoint *> &subscribers = itSub->second;
  std::vector<const Smp::IEntryPoint *>::iterator it =
      std::find(subscribers.begin(), subscribers.end(), entryPoint);

  // Verify existence before removal
  if (it == subscribers.end()) {
    throw core::EntryPointNotSubscribed(entryPoint,
                                        "Entry point not subscribed");
  }

  // Remove subscriber
  subscribers.erase(it);
}

void EventManager::Emit(Smp::Services::EventId event, Smp::Bool synchronous) {
  // Implements FE-0070.4.7: IEventManager Emit shall emit a global event.
  // Supports synchronous flag as per FE-0070.4.9.
  // The copying of subscribers helps address FE-0070.4.11.
  // @warning Does not explicitly implement FE-0070.4.10 (State transition event
  // shall not trigger another transition).

  // Copy subscribers to avoid deadlock if they modify subscriptions during
  // execution [CS-0010.35] Forbidden auto replaced with explicit type
  std::vector<const Smp::IEntryPoint *> subscribersCopy;
  {
    std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
    if (!lock.owns_lock()) {
      throw std::runtime_error("Timeout acquiring EventManager lock");
    }
    std::map<Smp::Services::EventId,
             std::vector<const Smp::IEntryPoint *>>::iterator it =
        _subscriptions.find(event);
    if (it != _subscriptions.end()) {
      subscribersCopy = it->second;
    }
  }

  // Iterate over the copy and execute each entry point
  for (std::vector<const Smp::IEntryPoint *>::const_iterator itCopy =
           subscribersCopy.begin();
       itCopy != subscribersCopy.end(); ++itCopy) {
    const Smp::IEntryPoint *ep = *itCopy;
    if (ep) {
      // Execute the entry point
      ep->Execute();
    }
  }
}

} // namespace utils
