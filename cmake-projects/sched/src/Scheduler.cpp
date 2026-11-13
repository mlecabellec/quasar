/**
 * @file Scheduler.cpp
 * @brief Implementation of the `sched::Scheduler` SMP service methods.
 *
 * @details Implements immediate, simulation-time, mission-time, and epoch-time
 * event scheduling, rescheduling, removal, and ordered timeline execution under
 * `std::timed_mutex` synchronization.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.44, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6
 * @compliance Implements `Scheduler` methods with null/time validation, timed mutex
 * RAII guards, explicit iterator types (no `auto`), unlocked callback dispatch, and
 * control-structure justifications.
 */

// Header under implementation and SMP event manager interface.
#include "sched/Scheduler.hpp"
#include <Smp/Services/IEventManager.h>

// Standard algorithms, limits, exceptions, and Quasar SMP exceptions.
#include <algorithm>
#include <core/StandardExceptions.hpp>
#include <limits>
#include <stdexcept>

namespace sched {

// Constructor for Scheduler: stores validated service dependencies and initializes base object.
Scheduler::Scheduler(Smp::Services::ITimeKeeper *timeKeeper,
                     Smp::Services::ILogger *logger)
    : core::Object("Scheduler", "SMP Scheduler Service", nullptr),
      _timeKeeper(timeKeeper), _logger(logger) {
  // [CS-0010.16] Reject null TimeKeeper or Logger dependencies immediately on construction.
  if (!timeKeeper || !logger) {
    throw std::invalid_argument("TimeKeeper and Logger services must be provided");
  }
}

Smp::ComponentStateKind Scheduler::GetState() const {
  return Smp::ComponentStateKind::CSK_Connected;
}

void Scheduler::Publish(Smp::IPublication *receiver) {}

void Scheduler::Configure(Smp::Services::ILogger *logger,
                          Smp::Services::ILinkRegistry *linkRegistry) {}

void Scheduler::Connect(Smp::ISimulator *simulator) {}

void Scheduler::Disconnect() {}

const Smp::Uuid &Scheduler::GetUuid() const {
  static Smp::Uuid uuid = {0, 0, 0, 0, 3}; // Generic Service UUID
  return uuid;
}

Smp::IField *Scheduler::GetField(Smp::String8 fullName) const {
  return nullptr;
}

const Smp::FieldCollection *Scheduler::GetFields() const { return nullptr; }

Smp::AnySimple Scheduler::GetSimpleValue(Smp::String8 fullName) const {
  return Smp::AnySimple();
}

void Scheduler::SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) {}

void Scheduler::GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                    Smp::AnySimple *values,
                                    Smp::UInt64 startIndex) const {}

void Scheduler::SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                    Smp::AnySimpleArray values,
                                    Smp::UInt64 startIndex) {}

Smp::Bool Scheduler::AddChild(Smp::IObject *child,
                              const Smp::ICollectionBase *collection) {
  return false;
}

Smp::Bool Scheduler::RemoveChild(Smp::IObject *child,
                                 const Smp::ICollectionBase *collection) {
  return false;
}

Smp::IObject *
Scheduler::IsChildInCollection(Smp::String8 child,
                               const Smp::ICollectionBase *collection) const {
  return nullptr;
}

Smp::IObject *Scheduler::GetChild(Smp::String8 name) const { return nullptr; }

Smp::Services::EventId Scheduler::CreateEventId() { return _nextEventId++; }

bool Scheduler::HasEvents() const {
  // Acquire timed lock with 1-second timeout to inspect pending event queues safely.
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Fail deterministically if scheduler lock acquisition timed out.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring scheduler lock in HasEvents");
  }
  return !_timeline.empty() || !_immediateEvents.empty();
}

Smp::Services::EventId
Scheduler::AddImmediateEvent(const Smp::IEntryPoint *entryPoint) {
  // [CS-0010.16] Reject null entry point callbacks prior to locking.
  if (!entryPoint) {
    throw std::invalid_argument("Entry point is null");
  }

  // Use RAII for mutex [CS-0010.22] bounded to a 1-second wait [CS-0010.26].
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout error if _mutex cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring scheduler lock in AddImmediateEvent");
  }

  // Initialize core immediate event metadata and current simulation dispatch timestamp.
  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.time = _timeKeeper->GetSimulationTime();
  evt.cycleTime = 0;

  // Initialize one-shot counters and sequence ordering key.
  evt.repeat = 0;
  evt.count = 0;
  evt.sequenceId = _nextSequenceId++;

  // Mark event as immediate simulation-domain (non-mission, non-epoch, non-Zulu).
  evt.isMissionTime = false;
  evt.isEpochTime = false;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  // Register event record and enqueue ID for immediate dispatch.
  _events[evt.id] = evt;
  _immediateEvents.push_back(evt.id);

  return evt.id;
}

Smp::Services::EventId
Scheduler::AddSimulationTimeEvent(const Smp::IEntryPoint *entryPoint,
                                  Smp::Duration simulationTime,
                                  Smp::Duration cycleTime, Smp::Int64 repeat) {
  // [CS-0010.16] Reject null entry point callbacks prior to locking.
  if (!entryPoint) {
    throw std::invalid_argument("Entry point is null");
  }

  // Validate non-negative target simulation time per SMP scheduler semantics.
  if (simulationTime < 0) {
    throw core::InvalidEventTime("Simulation time cannot be negative");
  }
  // Enforce strictly positive cycleTime whenever event repetition is requested.
  if (repeat != 0 && cycleTime <= 0) {
    throw core::InvalidCycleTime(
        "Cycle time must be positive for cyclic events");
  }

  // Use RAII for mutex [CS-0010.22] with 1-second timeout [CS-0010.26].
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout error if _mutex cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring scheduler lock in AddSimulationTimeEvent");
  }

  // Initialize core event identification and simulation target time.
  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.time = simulationTime;
  evt.cycleTime = cycleTime;

  // Initialize repeat policy, execution counter, and monotonic sequence order.
  evt.repeat = repeat;
  evt.count = 0;
  evt.sequenceId = _nextSequenceId++;

  // Clear domain flags because this event is native simulation-time scheduled.
  evt.isMissionTime = false;
  evt.isEpochTime = false;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  // Register event record and insert into ordered simulation timeline.
  _events[evt.id] = evt;
  ScheduleEvent(evt);

  return evt.id;
}

Smp::Services::EventId
Scheduler::AddMissionTimeEvent(const Smp::IEntryPoint *entryPoint,
                               Smp::Duration missionTime,
                               Smp::Duration cycleTime, Smp::Int64 repeat) {
  // [CS-0010.16] Reject null entry point callbacks prior to locking.
  if (!entryPoint) {
    throw std::invalid_argument("Entry point is null");
  }

  // Reject mission timestamps earlier than the current mission clock.
  if (missionTime < _timeKeeper->GetMissionTime()) {
    throw core::InvalidEventTime("Mission time in the past");
  }
  // Enforce strictly positive cycleTime whenever event repetition is requested.
  if (repeat != 0 && cycleTime <= 0) {
    throw core::InvalidCycleTime(
        "Cycle time must be positive for cyclic events");
  }

  // Use RAII for mutex [CS-0010.22] with 1-second timeout [CS-0010.26].
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout error if _mutex cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring scheduler lock in AddMissionTimeEvent");
  }

  // Initialize event identification and preserve original requested mission time.
  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.missionTime = missionTime;

  // Map mission time to simulation time using current epoch/simulation offset and mission start.
  evt.time = missionTime -
             (_timeKeeper->GetEpochTime() - _timeKeeper->GetSimulationTime()) +
             _timeKeeper->GetMissionStartTime();

  // Set recurrence parameters and monotonic sequence ID.
  evt.cycleTime = cycleTime;
  evt.repeat = repeat;
  evt.count = 0;
  evt.sequenceId = _nextSequenceId++;

  // Flag event as mission-time based.
  evt.isMissionTime = true;
  evt.isEpochTime = false;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  // Register event record and insert into ordered simulation timeline.
  _events[evt.id] = evt;
  ScheduleEvent(evt);

  return evt.id;
}

Smp::Services::EventId
Scheduler::AddEpochTimeEvent(const Smp::IEntryPoint *entryPoint,
                             Smp::DateTime epochTime, Smp::Duration cycleTime,
                             Smp::Int64 repeat) {
  // [CS-0010.16] Reject null entry point callbacks prior to locking.
  if (!entryPoint) {
    throw std::invalid_argument("Entry point is null");
  }

  // Reject epoch timestamps earlier than the current simulation epoch clock.
  if (epochTime < _timeKeeper->GetEpochTime()) {
    throw core::InvalidEventTime("Epoch time in the past");
  }
  // Enforce strictly positive cycleTime whenever event repetition is requested.
  if (repeat != 0 && cycleTime <= 0) {
    throw core::InvalidCycleTime("Cycle time must be positive");
  }

  // Use RAII for mutex [CS-0010.22] with 1-second timeout [CS-0010.26].
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout error if _mutex cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring scheduler lock in AddEpochTimeEvent");
  }

  // Initialize event identification and preserve original requested epoch time.
  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.epochTime = epochTime;

  // Map epoch time to simulation time using current epoch-to-simulation offset.
  evt.time = epochTime -
             (_timeKeeper->GetEpochTime() - _timeKeeper->GetSimulationTime());

  // Set recurrence parameters and monotonic sequence ID.
  evt.cycleTime = cycleTime;
  evt.repeat = repeat;
  evt.count = 0;
  evt.sequenceId = _nextSequenceId++;

  // Flag event as epoch-time based.
  evt.isMissionTime = false;
  evt.isEpochTime = true;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  // Register event record and insert into ordered simulation timeline.
  _events[evt.id] = evt;
  ScheduleEvent(evt);

  return evt.id;
}

Smp::Services::EventId
Scheduler::AddZuluTimeEvent(const Smp::IEntryPoint *entryPoint,
                            Smp::DateTime zuluTime, Smp::Duration cycleTime,
                            Smp::Int64 repeat) {
  // Not fully implemented, placeholder
  return -1;
}

Smp::Services::EventId Scheduler::AddRelativeZuluTimeEvent(
    const Smp::IEntryPoint *entryPoint, Smp::Duration deltaZuluTime,
    Smp::Duration cycleTime, Smp::Int64 repeat) {
  // Not fully implemented, placeholder
  return -1;
}

void Scheduler::ScheduleEvent(const SchedulerEvent &evt) {
  // Add to simulation timeline only if event is not a wall-clock Zulu event.
  if (!evt.isZuluTime && !evt.isRelativeZulu) {
    _timeline.insert({evt.time, evt.id});
  }
}

void Scheduler::RemoveFromTimeline(Smp::Services::EventId id) {
  // Linear search in timeline for matching event ID, bounded by active _timeline size.
  // [CS-0010.35] Forbidden auto replaced with explicit type
  for (std::multimap<Smp::Duration, Smp::Services::EventId>::iterator it = _timeline.begin(); it != _timeline.end();) {
    // Erase matching timeline entry and advance iterator safely.
    if (it->second == id) {
      it = _timeline.erase(it);
    } else {
      // Non-matching entry; advance iterator to next timeline node.
      ++it;
    }
  }
}

void Scheduler::RemoveEvent(Smp::Services::EventId event) {
  // Use RAII for mutex [CS-0010.22] bounded to 1-second wait [CS-0010.26].
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Fail fast if lock acquisition timed out under contention.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring scheduler lock in RemoveEvent");
  }

  // Find event in primary ownership collection.
  std::map<Smp::Services::EventId, SchedulerEvent>::iterator it = _events.find(event);
  // Reject unknown event IDs per SMP InvalidEventId contract.
  if (it == _events.end()) {
    throw core::InvalidEventId(event);
  }

  // Remove from all tracking structures
  RemoveFromTimeline(event);

  // [CS-0010.35] Forbidden auto replaced with explicit type
  std::vector<Smp::Services::EventId>::iterator itImm =
      std::find(_immediateEvents.begin(), _immediateEvents.end(), event);
  // Remove event ID from immediate queue if present.
  if (itImm != _immediateEvents.end()) {
    _immediateEvents.erase(itImm);
  }

  _events.erase(it);
}

Smp::Bool Scheduler::IsEventScheduled(Smp::Services::EventId event) const {
  // Acquire timed lock to check existence in event map safely.
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout exception if lock cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring scheduler lock in IsEventScheduled");
  }
  return _events.find(event) != _events.end();
}

void Scheduler::SetEventSimulationTime(Smp::Services::EventId event,
                                       Smp::Duration simulationTime) {
  // Use RAII for mutex [CS-0010.22] with 1-second timeout [CS-0010.26].
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout exception if lock cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring scheduler lock in SetEventSimulationTime");
  }

  // Find and update event simulation time.
  std::map<Smp::Services::EventId, SchedulerEvent>::iterator it = _events.find(event);
  // Reject unknown event identifiers per SMP contract.
  if (it == _events.end())
    throw core::InvalidEventId(event);

  // Verify the event belongs to the simulation-time domain before rescheduling.
  if (it->second.isMissionTime || it->second.isEpochTime ||
      it->second.isZuluTime) {
    throw core::InvalidEventId(event);
  }

  // Remove if time is negative (standard behavior); unlock first to avoid non-recursive deadlock.
  if (simulationTime < 0) {
    lock.unlock();
    RemoveEvent(event);
    // Re-acquire lock to restore RAII ownership invariants prior to return.
    if (!lock.try_lock_for(std::chrono::seconds(1))) {
      throw std::runtime_error("Timeout re-acquiring scheduler lock in SetEventSimulationTime");
    }
    return;
  }

  // Reschedule with new time
  RemoveFromTimeline(event);
  it->second.time = simulationTime;
  ScheduleEvent(it->second);
}

void Scheduler::SetEventMissionTime(Smp::Services::EventId event,
                                    Smp::Duration missionTime) {
}
void Scheduler::SetEventEpochTime(Smp::Services::EventId event,
                                  Smp::DateTime epochTime) {
}
void Scheduler::SetEventZuluTime(Smp::Services::EventId event,
                                 Smp::DateTime zuluTime) {
}
void Scheduler::SetEventCycleTime(Smp::Services::EventId event,
                                  Smp::Duration cycleTime) {
}
void Scheduler::SetEventRepeat(Smp::Services::EventId event,
                               Smp::Int64 repeat) {
}

Smp::Services::EventId Scheduler::GetCurrentEventId() const {
  return _currentEventId;
}

Smp::Duration Scheduler::GetNextScheduledEventTime() const {
  // Acquire timed lock to read the earliest scheduled simulation timestamp.
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout error if lock cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring scheduler lock in GetNextScheduledEventTime");
  }
  // Return the key of the first timeline entry if any timeline events exist.
  if (!_timeline.empty()) {
    return _timeline.begin()->first;
  }
  return std::numeric_limits<Smp::Duration>::max();
}

Smp::Duration Scheduler::ExecuteNextEvent() {
  // Acquire timed lock to inspect immediate and timeline event queues.
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Abort step if lock cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring scheduler lock in ExecuteNextEvent");
  }

  // 1. Process immediate events first in FIFO order.
  if (!_immediateEvents.empty()) {
    Smp::Services::EventId id = _immediateEvents.front();
    _immediateEvents.erase(_immediateEvents.begin());

    std::map<Smp::Services::EventId, SchedulerEvent>::iterator it = _events.find(id);
    // Execute immediate event only if its metadata record still exists in _events.
    if (it != _events.end()) {
      _currentEventId = id;
      const Smp::IEntryPoint* ep = it->second.entryPoint;
      // Guard against null entry point before unlocking and invoking Execute().
      if (ep) {
        // Unlock while executing to prevent deadlocks and allow additions
        lock.unlock();
        ep->Execute();
        // Re-acquire timed lock after callback completes to clean up state safely.
        if (!lock.try_lock_for(std::chrono::seconds(1))) {
          throw std::runtime_error("Timeout re-acquiring scheduler lock in ExecuteNextEvent (immediate)");
        }
      }
      _currentEventId = -1;
      _events.erase(id); // Immediate events are one-shot

      return _timeKeeper->GetSimulationTime();
    }
  }

  // 2. Process timeline events in chronological simulation-time order.
  if (!_timeline.empty()) {
    std::multimap<Smp::Duration, Smp::Services::EventId>::iterator itTimeline = _timeline.begin();
    Smp::Duration time = itTimeline->first;
    Smp::Services::EventId id = itTimeline->second;

    // Advance simulation time to the event time outside the scheduler lock.
    lock.unlock();
    // Advance TimeKeeper only when the event timestamp is strictly ahead of current time.
    if (time > _timeKeeper->GetSimulationTime()) {
      _timeKeeper->SetSimulationTime(time);
    }
    // Re-acquire timed lock prior to mutating _timeline and inspecting _events.
    if (!lock.try_lock_for(std::chrono::seconds(1))) {
      throw std::runtime_error("Timeout re-acquiring scheduler lock in ExecuteNextEvent (timeline advance)");
    }

    // Find and execute the event if it was not cancelled while unlocked.
    std::map<Smp::Services::EventId, SchedulerEvent>::iterator itEvt = _events.find(id);
    // Dispatch event callback and apply repeat/cleanup policy when event record is valid.
    if (itEvt != _events.end()) {
      _timeline.erase(itTimeline);

      _currentEventId = id;
      const Smp::IEntryPoint* ep = itEvt->second.entryPoint;

      // Unlock for execution so callbacks can inspect or modify the scheduler.
      lock.unlock();
      // Invoke entry point only if pointer is non-null.
      if (ep) {
        ep->Execute();
      }
      // Re-acquire timed lock with 1-second bound before updating repeat counters.
      if (!lock.try_lock_for(std::chrono::seconds(1))) {
        throw std::runtime_error("Timeout re-acquiring scheduler lock in ExecuteNextEvent (timeline execute)");
      }

      _currentEventId = -1;

      // Handle cyclic events based on updated execution count and repeat limit.
      SchedulerEvent& evt = _events[id];
      evt.count++;
      // Reschedule if repeat is infinite (< 0) or remaining cycles exist; else erase finished event.
      if (evt.repeat < 0 || evt.count <= evt.repeat) {
        // Reschedule for next cycle
        evt.time += evt.cycleTime;
        ScheduleEvent(evt);
      } else {
        // Cleanup finished event when repeat count is exhausted.
        _events.erase(id);
      }

      return time;
    }
  }

  return -1; // No events executed
}

} // namespace sched
