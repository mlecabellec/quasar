#include "sched/Scheduler.hpp"
#include <Smp/Services/IEventManager.h>
#include <algorithm>
#include <core/StandardExceptions.hpp>
#include <limits>
#include <stdexcept>

namespace sched {

/**
 * @brief Constructor for Scheduler.
 * @param timeKeeper TimeKeeper service pointer.
 * @param logger Logger service pointer.
 */
Scheduler::Scheduler(Smp::Services::ITimeKeeper *timeKeeper,
                     Smp::Services::ILogger *logger)
    : core::Object("Scheduler", "SMP Scheduler Service", nullptr),
      _timeKeeper(timeKeeper), _logger(logger) {
  // [CS-0010.16] Check for null pointers
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
  // Check if any events are pending execution
  std::lock_guard<std::mutex> lock(_mutex);
  return !_timeline.empty() || !_immediateEvents.empty();
}

Smp::Services::EventId
Scheduler::AddImmediateEvent(const Smp::IEntryPoint *entryPoint) {
  // [CS-0010.16] Check for null pointers
  if (!entryPoint) {
    throw std::invalid_argument("Entry point is null");
  }

  // Use RAII for mutex [CS-0010.22]
  std::lock_guard<std::mutex> lock(_mutex);

  // Initialize event structure
  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.time = _timeKeeper->GetSimulationTime();
  evt.cycleTime = 0;
  evt.repeat = 0;
  evt.count = 0;
  evt.sequenceId = _nextSequenceId++;
  evt.isMissionTime = false;
  evt.isEpochTime = false;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  // Add to collections
  _events[evt.id] = evt;
  _immediateEvents.push_back(evt.id);

  return evt.id;
}

Smp::Services::EventId
Scheduler::AddSimulationTimeEvent(const Smp::IEntryPoint *entryPoint,
                                  Smp::Duration simulationTime,
                                  Smp::Duration cycleTime, Smp::Int64 repeat) {
  // [CS-0010.16] Check for null pointers
  if (!entryPoint) {
    throw std::invalid_argument("Entry point is null");
  }

  // Validate timing constraints
  if (simulationTime < 0) {
    throw core::InvalidEventTime("Simulation time cannot be negative");
  }
  if (repeat != 0 && cycleTime <= 0) {
    throw core::InvalidCycleTime(
        "Cycle time must be positive for cyclic events");
  }

  // Use RAII for mutex [CS-0010.22]
  std::lock_guard<std::mutex> lock(_mutex);

  // Initialize event structure
  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.time = simulationTime;
  evt.cycleTime = cycleTime;
  evt.repeat = repeat;
  evt.count = 0;
  evt.sequenceId = _nextSequenceId++;
  evt.isMissionTime = false;
  evt.isEpochTime = false;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  // Add to collections
  _events[evt.id] = evt;
  ScheduleEvent(evt);

  return evt.id;
}

Smp::Services::EventId
Scheduler::AddMissionTimeEvent(const Smp::IEntryPoint *entryPoint,
                               Smp::Duration missionTime,
                               Smp::Duration cycleTime, Smp::Int64 repeat) {
  // [CS-0010.16] Check for null pointers
  if (!entryPoint) {
    throw std::invalid_argument("Entry point is null");
  }

  // Validate timing constraints
  if (missionTime < _timeKeeper->GetMissionTime()) {
    throw core::InvalidEventTime("Mission time in the past");
  }
  if (repeat != 0 && cycleTime <= 0) {
    throw core::InvalidCycleTime(
        "Cycle time must be positive for cyclic events");
  }

  // Use RAII for mutex [CS-0010.22]
  std::lock_guard<std::mutex> lock(_mutex);

  // Initialize event structure
  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.missionTime = missionTime;
  
  // Map mission time to simulation time
  evt.time = missionTime -
             (_timeKeeper->GetEpochTime() - _timeKeeper->GetSimulationTime()) +
             _timeKeeper->GetMissionStartTime();

  evt.cycleTime = cycleTime;
  evt.repeat = repeat;
  evt.count = 0;
  evt.sequenceId = _nextSequenceId++;
  evt.isMissionTime = true;
  evt.isEpochTime = false;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  // Add to collections
  _events[evt.id] = evt;
  ScheduleEvent(evt);

  return evt.id;
}

Smp::Services::EventId
Scheduler::AddEpochTimeEvent(const Smp::IEntryPoint *entryPoint,
                             Smp::DateTime epochTime, Smp::Duration cycleTime,
                             Smp::Int64 repeat) {
  // [CS-0010.16] Check for null pointers
  if (!entryPoint) {
    throw std::invalid_argument("Entry point is null");
  }

  // Validate timing constraints
  if (epochTime < _timeKeeper->GetEpochTime()) {
    throw core::InvalidEventTime("Epoch time in the past");
  }
  if (repeat != 0 && cycleTime <= 0) {
    throw core::InvalidCycleTime("Cycle time must be positive");
  }

  // Use RAII for mutex [CS-0010.22]
  std::lock_guard<std::mutex> lock(_mutex);

  // Initialize event structure
  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.epochTime = epochTime;
  
  // Map epoch time to simulation time
  evt.time = epochTime -
             (_timeKeeper->GetEpochTime() - _timeKeeper->GetSimulationTime());
  evt.cycleTime = cycleTime;
  evt.repeat = repeat;
  evt.count = 0;
  evt.sequenceId = _nextSequenceId++;
  evt.isMissionTime = false;
  evt.isEpochTime = true;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  // Add to collections
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
  // Add to simulation timeline if not a Zulu event
  if (!evt.isZuluTime && !evt.isRelativeZulu) {
    _timeline.insert({evt.time, evt.id});
  }
}

void Scheduler::RemoveFromTimeline(Smp::Services::EventId id) {
  // Linear search in timeline for matching event ID
  // [CS-0010.35] Forbidden auto replaced with explicit type
  for (std::multimap<Smp::Duration, Smp::Services::EventId>::iterator it = _timeline.begin(); it != _timeline.end();) {
    if (it->second == id) {
      it = _timeline.erase(it);
    } else {
      ++it;
    }
  }
}

void Scheduler::RemoveEvent(Smp::Services::EventId event) {
  // Use RAII for mutex [CS-0010.22]
  std::lock_guard<std::mutex> lock(_mutex);
  
  // Find event in collection
  std::map<Smp::Services::EventId, SchedulerEvent>::iterator it = _events.find(event);
  if (it == _events.end()) {
    throw core::InvalidEventId(event);
  }

  // Remove from all tracking structures
  RemoveFromTimeline(event);

  // [CS-0010.35] Forbidden auto replaced with explicit type
  std::vector<Smp::Services::EventId>::iterator itImm =
      std::find(_immediateEvents.begin(), _immediateEvents.end(), event);
  if (itImm != _immediateEvents.end()) {
    _immediateEvents.erase(itImm);
  }

  _events.erase(it);
}

Smp::Bool Scheduler::IsEventScheduled(Smp::Services::EventId event) const {
  // Check existence in event map
  std::lock_guard<std::mutex> lock(_mutex);
  return _events.find(event) != _events.end();
}

void Scheduler::SetEventSimulationTime(Smp::Services::EventId event,
                                       Smp::Duration simulationTime) {
  // Use RAII for mutex [CS-0010.22]
  std::lock_guard<std::mutex> lock(_mutex);
  
  // Find and update event simulation time
  std::map<Smp::Services::EventId, SchedulerEvent>::iterator it = _events.find(event);
  if (it == _events.end())
    throw core::InvalidEventId(event);
    
  // Check if it's the correct kind of event
  if (it->second.isMissionTime || it->second.isEpochTime ||
      it->second.isZuluTime) {
    throw core::InvalidEventId(event);
  }

  // Remove if time is negative (standard behavior)
  if (simulationTime < 0) {
    _mutex.unlock();
    RemoveEvent(event);
    _mutex.lock();
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
  // Return the time of the first event in the timeline
  std::lock_guard<std::mutex> lock(_mutex);
  if (!_timeline.empty()) {
    return _timeline.begin()->first;
  }
  return std::numeric_limits<Smp::Duration>::max();
}

Smp::Duration Scheduler::ExecuteNextEvent() {
  // Process scheduled events
  std::lock_guard<std::mutex> lock(_mutex);

  // 1. Process immediate events first
  if (!_immediateEvents.empty()) {
    Smp::Services::EventId id = _immediateEvents.front();
    _immediateEvents.erase(_immediateEvents.begin());

    std::map<Smp::Services::EventId, SchedulerEvent>::iterator it = _events.find(id);
    if (it != _events.end()) {
      _currentEventId = id;
      const Smp::IEntryPoint* ep = it->second.entryPoint;
      if (ep) {
        // Unlock while executing to prevent deadlocks and allow additions
        _mutex.unlock();
        ep->Execute();
        _mutex.lock();
      }
      _currentEventId = -1;
      _events.erase(id); // Immediate events are one-shot

      return _timeKeeper->GetSimulationTime();
    }
  }

  // 2. Process timeline events
  if (!_timeline.empty()) {
    std::multimap<Smp::Duration, Smp::Services::EventId>::iterator itTimeline = _timeline.begin();
    Smp::Duration time = itTimeline->first;
    Smp::Services::EventId id = itTimeline->second;

    // Advance simulation time to the event time
    _mutex.unlock();
    if (time > _timeKeeper->GetSimulationTime()) {
      _timeKeeper->SetSimulationTime(time);
    }
    _mutex.lock();

    // Find and execute the event
    std::map<Smp::Services::EventId, SchedulerEvent>::iterator itEvt = _events.find(id);
    if (itEvt != _events.end()) {
      _timeline.erase(itTimeline);

      _currentEventId = id;
      const Smp::IEntryPoint* ep = itEvt->second.entryPoint;

      // Unlock for execution
      _mutex.unlock();
      if (ep) {
        ep->Execute();
      }
      _mutex.lock();

      _currentEventId = -1;

      // Handle cyclic events
      SchedulerEvent& evt = _events[id];
      evt.count++;
      if (evt.repeat < 0 || evt.count <= evt.repeat) {
        // Reschedule for next cycle
        evt.time += evt.cycleTime;
        ScheduleEvent(evt);
      } else {
        // Cleanup finished event
        _events.erase(id);
      }

      return time;
    }
  }

  return -1; // No events executed
}

} // namespace sched
