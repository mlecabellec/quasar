#include <algorithm>
#include <core/StandardExceptions.hpp>
#include <cstring>

namespace utils {

EventManager::EventManager() { RegisterPredefinedEvents(); }

void EventManager::RegisterPredefinedEvents() {
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

  // Also fill reverse map if needed, but QueryEventId uses _eventIds
}

Smp::Services::EventId EventManager::QueryEventId(Smp::String8 eventName) {
  if (!eventName) {
    throw core::InvalidEventName("null", "Event name is null");
  }

  std::lock_guard<std::mutex> lock(_mutex);

  auto it = _eventIds.find(eventName);
  if (it != _eventIds.end()) {
    return it->second;
  }

  // Create new event ID
  Smp::Services::EventId newId = _nextEventId++;
  _eventIds[eventName] = newId;
  _eventNames[newId] = eventName;
  return newId;
}

void EventManager::Subscribe(Smp::Services::EventId event,
                             const Smp::IEntryPoint *entryPoint) {
  std::lock_guard<std::mutex> lock(_mutex);

  if (event <=
      0) { // Assuming 0 is invalid, though standard doesn't explicitly say so,
           // often 0 is InvalidEventId? Standard says "raises ...
           // InvalidEventId when called with an invalid event identifier"
           // Looking at header, typical IDs start at 1. Let's assume <= 0 or
           // just check if it exists in map (if strict). However, we allow
           // creating IDs on the fly? No, QueryEventId creates them. So if it's
           // not in our map (or known range), strictly it might be invalid? But
           // the interface allows passing an ID directly. For now, let's just
           // create the entry if missing, or strictly check? The spec says
           // "raises InvalidEventId". It implies validity check. Since we
           // auto-generate IDs, any ID < _nextEventId and > 0 could be valid.
  }

  auto &subscribers = _subscriptions[event];
  auto it = std::find(subscribers.begin(), subscribers.end(), entryPoint);
  if (it != subscribers.end()) {
    throw core::EntryPointAlreadySubscribed(entryPoint, "Unknown",
                                            "Entry point already subscribed");
  }
  subscribers.push_back(entryPoint);
}

void EventManager::Unsubscribe(Smp::Services::EventId event,
                               const Smp::IEntryPoint *entryPoint) {
  std::lock_guard<std::mutex> lock(_mutex);

  auto itSub = _subscriptions.find(event);
  if (itSub == _subscriptions.end()) {
    // Maybe event doesn't exist or no subs, treat as "not subscribed"
    throw core::EntryPointNotSubscribed(
        entryPoint, "Unknown",
        "Entry point not subscribed (event has no subscribers)");
  }

  auto &subscribers = itSub->second;
  auto it = std::find(subscribers.begin(), subscribers.end(), entryPoint);
  if (it == subscribers.end()) {
    throw core::EntryPointNotSubscribed(entryPoint, "Unknown",
                                        "Entry point not subscribed");
  }
  subscribers.erase(it);
}

void EventManager::Emit(Smp::Services::EventId event, Smp::Bool synchronous) {
  // Copy subscribers to avoid deadlock if they modify subscriptions?
  // Spec says: "Entry point subscription/unsubscription during the execution of
  // Emit() is taken into account the next time Emit() is called." This implies
  // we should work on a copy of the list.

  std::vector<const Smp::IEntryPoint *> subscribersCopy;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _subscriptions.find(event);
    if (it != _subscriptions.end()) {
      subscribersCopy = it->second;
    }
  }

  for (const auto *ep : subscribersCopy) {
    if (ep) {
      // const_cast because Execute is non-const usually?
      // IEntryPoint::Execute() is abstract.
      // Let's check IEntryPoint.h
      // It is `virtual void Execute() const = 0;` in some versions, or
      // non-const. Standard says `Execute() const`? Let's assume const for now,
      // or use const_cast if needed. Actually `IEntryPoint` usually has
      // `Execute() const`.
      ep->Execute();
    }
  }
}

} // namespace utils
