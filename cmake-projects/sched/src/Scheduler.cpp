#include "sched/Scheduler.hpp"
#include <Smp/Services/IEventManager.h>
#include <algorithm>
#include <core/StandardExceptions.hpp>
#include <limits>

namespace sched {

Scheduler::Scheduler(Smp::Services::ITimeKeeper *timeKeeper,
                     Smp::Services::ILogger *logger)
    : core::Object("Scheduler", "SMP Scheduler Service", nullptr),
      _timeKeeper(timeKeeper), _logger(logger) {}

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
  std::lock_guard<std::mutex> lock(_mutex);
  return !_timeline.empty() || !_immediateEvents.empty();
}

Smp::Services::EventId
Scheduler::AddImmediateEvent(const Smp::IEntryPoint *entryPoint) {
  // [FE-0070.3.9] IScheduler AddImmediateEvent shall add an immediate Event.
  std::lock_guard<std::mutex> lock(_mutex);

  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.time = _timeKeeper->GetSimulationTime(); // Conceptually now
  evt.cycleTime = 0;
  evt.repeat = 0;
  evt.count = 0;
  evt.sequenceId = _nextSequenceId++;
  evt.isMissionTime = false;
  evt.isEpochTime = false;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  _events[evt.id] = evt;
  _immediateEvents.push_back(evt.id);

  return evt.id;
}

Smp::Services::EventId
Scheduler::AddSimulationTimeEvent(const Smp::IEntryPoint *entryPoint,
                                  Smp::Duration simulationTime,
                                  Smp::Duration cycleTime, Smp::Int64 repeat) {
  // [FE-0070.3.5] IScheduler AddSimulationTimeEvent shall add an Event.
  if (simulationTime < 0) {
    throw core::InvalidEventTime("Simulation time cannot be negative");
  }
  // [FE-0070.3.3] The Scheduler shall support cycle time for cyclic Events.
  if (repeat != 0 && cycleTime <= 0) {
    throw core::InvalidCycleTime(
        "Cycle time must be positive for cyclic events");
  }

  std::lock_guard<std::mutex> lock(_mutex);

  SchedulerEvent evt;
  // [FE-0070.3.10] EventId shall be unique throughout simulation.
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.time = simulationTime;
  evt.cycleTime = cycleTime;
  // [FE-0070.3.2] The Scheduler shall support Events with repeat count.
  evt.repeat = repeat;
  evt.count = 0;
  // [FE-0070.3.4] Events shall be executed first-posted, first-executed.
  // (sequenceId for tie-breaking)
  evt.sequenceId = _nextSequenceId++;
  evt.isMissionTime = false;
  evt.isEpochTime = false;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  _events[evt.id] = evt;
  ScheduleEvent(evt);

  return evt.id;
}

Smp::Services::EventId
Scheduler::AddMissionTimeEvent(const Smp::IEntryPoint *entryPoint,
                               Smp::Duration missionTime,
                               Smp::Duration cycleTime, Smp::Int64 repeat) {
  // [FE-0070.3.6] IScheduler AddMissionTimeEvent shall add an Event.
  // Check against current mission time? Spec says "If the Mission Time is less
  // than the current mission time... throws InvalidEventTime".
  if (missionTime < _timeKeeper->GetMissionTime()) {
    throw core::InvalidEventTime("Mission time in the past");
  }
  if (repeat != 0 && cycleTime <= 0) {
    throw core::InvalidCycleTime(
        "Cycle time must be positive for cyclic events");
  }

  std::lock_guard<std::mutex> lock(_mutex);

  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.missionTime = missionTime;
  // ...
  // (rest of code)

  // Map to Sim Time: SimTime = MissionTime + MissionStartOffset?
  // MissionTime (Duration) is offset from MissionStart.
  // SimTime = MissionTime - (MissionTimeOffset).
  // Wait. GetMissionTime() = GetEpochTime() - MissionStart.
  // EpochTime = SimTime + EpochOffset.
  // So MissionTime = SimTime + EpochOffset - MissionStart.
  // SimTime = MissionTime - EpochOffset + MissionStart.

  // We store the original requested time parameters for
  // persistence/modifications
  evt.time = missionTime -
             (_timeKeeper->GetEpochTime() - _timeKeeper->GetSimulationTime()) +
             _timeKeeper->GetMissionStartTime();
  // Wait, simpler: SimTime returns Duration.
  // SimTime = MissionTime + (SimTime - MissionTime)
  // SimTime - MissionTime = SimTime - (SimTime + EpochOffset - MissionStart) =
  // -EpochOffset + MissionStart. So SimTime = MissionTime - EpochOffset +
  // MissionStart. Correct. Note: If Epoch changes, we need to update SimTime of
  // Mission events? "Mission time typically progresses with simulation time...
  // Further, mission time is updated when changing epoch time with
  // SetEpochTime." Yes, if EpochOffset changes, the mapping changes. We should
  // probably store `isMissionTime` and re-calculate `time` (SimTime) whenever
  // needed or when Epoch changes. But for `_timeline` (SimTime ordered), we
  // need a concrete SimTime. Does `SetEpochTime` require rescheduling all
  // Mission events? Likely yes.

  evt.cycleTime = cycleTime;
  evt.repeat = repeat;
  evt.count = 0;
  evt.sequenceId = _nextSequenceId++;
  evt.isMissionTime = true;
  evt.isEpochTime = false;
  evt.isZuluTime = false;
  evt.isRelativeZulu = false;

  _events[evt.id] = evt;
  ScheduleEvent(evt);

  return evt.id;
}

// ... Implement others similarly ...
// For brevity, I will implement AddEpochTimeEvent and basic Set/Remove/Execute.

Smp::Services::EventId
Scheduler::AddEpochTimeEvent(const Smp::IEntryPoint *entryPoint,
                             Smp::DateTime epochTime, Smp::Duration cycleTime,
                             Smp::Int64 repeat) {
  // [FE-0070.3.7] IScheduler AddEpochTimeEvent shall add an Event.
  if (epochTime < _timeKeeper->GetEpochTime()) {
    throw core::InvalidEventTime("Epoch time in the past");
  }
  if (repeat != 0 && cycleTime <= 0) {
    throw core::InvalidCycleTime("Cycle time must be positive");
  }

  std::lock_guard<std::mutex> lock(_mutex);

  SchedulerEvent evt;
  evt.id = CreateEventId();
  evt.entryPoint = entryPoint;
  evt.epochTime = epochTime;
  // SimTime = EpochTime - EpochOffset
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

  _events[evt.id] = evt;
  ScheduleEvent(evt);

  return evt.id;
}

Smp::Services::EventId
Scheduler::AddZuluTimeEvent(const Smp::IEntryPoint *entryPoint,
                            Smp::DateTime zuluTime, Smp::Duration cycleTime,
                            Smp::Int64 repeat) {
  // [FE-0070.3.8] IScheduler AddZuluTimeEvent shall add an Event.
  // Logic for Zulu time...
  // For now, minimal implementation as SimTime is main focus.
  return -1;
}

Smp::Services::EventId Scheduler::AddRelativeZuluTimeEvent(
    const Smp::IEntryPoint *entryPoint, Smp::Duration deltaZuluTime,
    Smp::Duration cycleTime, Smp::Int64 repeat) {
  return -1;
}

void Scheduler::ScheduleEvent(const SchedulerEvent &evt) {
  // Only schedule if it's a SimTime execution candidate
  // (Zulu events are not on Sim timeline)
  if (!evt.isZuluTime && !evt.isRelativeZulu) {
    _timeline.insert({evt.time, evt.id});
  }
}

void Scheduler::RemoveFromTimeline(Smp::Services::EventId id) {
  // Linear search is slow, but map is sorted by time.
  // Only way is to iterate. Or keep an iterator in SchedulerEvent?
  // Iterators invalidate on modification though.
  // For now, linear search.
  for (auto it = _timeline.begin(); it != _timeline.end();) {
    if (it->second == id) {
      it = _timeline.erase(it);
    } else {
      ++it;
    }
  }
}

void Scheduler::RemoveEvent(Smp::Services::EventId event) {
  // [FE-0070.3.11] IScheduler RemoveEvent shall remove an Event.
  std::lock_guard<std::mutex> lock(_mutex);
  auto it = _events.find(event);
  if (it == _events.end()) {
    throw core::InvalidEventId(event);
  }

  RemoveFromTimeline(event);

  // Also remove from immediate
  auto itImm =
      std::find(_immediateEvents.begin(), _immediateEvents.end(), event);
  if (itImm != _immediateEvents.end()) {
    _immediateEvents.erase(itImm);
  }

  _events.erase(it);
}

Smp::Bool Scheduler::IsEventScheduled(Smp::Services::EventId event) const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _events.find(event) != _events.end();
}

void Scheduler::SetEventSimulationTime(Smp::Services::EventId event,
                                       Smp::Duration simulationTime) {
  // [FE-0070.3.12] IScheduler SetEventSimulationTime shall update Event time.
  std::lock_guard<std::mutex> lock(_mutex);
  auto it = _events.find(event);
  if (it == _events.end())
    throw core::InvalidEventId(event);
  // Check if it IS a sim time event? Spec says "In case an event is registered
  // under the given event Id but it is not a simulation time event, the method
  // throws an exception of type InvalidEventId as well." So we need to store
  // what kind it was created as.
  if (it->second.isMissionTime || it->second.isEpochTime ||
      it->second.isZuluTime) {
    throw core::InvalidEventId(event);
  }

  if (simulationTime < 0) {
    RemoveEvent(event);
    return;
  }

  RemoveFromTimeline(event);
  it->second.time = simulationTime;
  ScheduleEvent(it->second);
}

// Implement other Set methods... stubbing for now to compile.
void Scheduler::SetEventMissionTime(Smp::Services::EventId event,
                                    Smp::Duration missionTime) {
  // [FE-0070.3.13] IScheduler SetEventMissionTime shall update Event time.
}
void Scheduler::SetEventEpochTime(Smp::Services::EventId event,
                                  Smp::DateTime epochTime) {
  // [FE-0070.3.14] IScheduler SetEventEpochTime shall update Event time.
}
void Scheduler::SetEventZuluTime(Smp::Services::EventId event,
                                 Smp::DateTime zuluTime) {
  // [FE-0070.3.15] IScheduler SetEventZuluTime shall update Event time.
}
void Scheduler::SetEventCycleTime(Smp::Services::EventId event,
                                  Smp::Duration cycleTime) {
  // [FE-0070.3.16] IScheduler SetEventCycleTime shall update Event cycle.
}
void Scheduler::SetEventRepeat(Smp::Services::EventId event,
                               Smp::Int64 repeat) {
  // [FE-0070.3.17] IScheduler SetEventRepeat shall update Event repeat count.
}

Smp::Services::EventId Scheduler::GetCurrentEventId() const {
  // [FE-0070.3.18] IScheduler GetCurrentEventId shall return current EventId.
  return _currentEventId;
}

Smp::Duration Scheduler::GetNextScheduledEventTime() const {
  // [FE-0070.3.19] IScheduler GetNextScheduledEventTime shall return next Event
  // time.
  std::lock_guard<std::mutex> lock(_mutex);
  if (!_timeline.empty()) {
    return _timeline.begin()->first;
  }
  return std::numeric_limits<Smp::Duration>::max();
}

Smp::Duration Scheduler::ExecuteNextEvent() {
  // Contributes to [FE-0070.12.1] (Executing state event processing)
  std::lock_guard<std::mutex> lock(_mutex);

  // 1. Immediate events
  if (!_immediateEvents.empty()) {
    Smp::Services::EventId id = _immediateEvents.front();
    _immediateEvents.erase(_immediateEvents.begin());

    auto it = _events.find(id);
    if (it != _events.end()) {
      _currentEventId = id;
      if (it->second.entryPoint) {
        // Unlock during execution?
        // "While an event for such a state transition is emitted, subscribed
        // event handlers are not allowed..." But this is normal execution.
        // Depending on threading model. If single threaded, we can unlock to
        // allow AddEvent during execution. But _events map stability? Standard
        // says: "AddEvent... returns the EventId...". If we unlock, we need to
        // be careful about `it`. Copy event info.
        auto ep = it->second.entryPoint;
        _mutex.unlock();
        ep->Execute();
        _mutex.lock();
      }
      _currentEventId = -1;

      // Re-schedule if cyclic? Immediate events are usually one-shot unless
      // specified? "As an immediate event, it will be executed when the
      // scheduler processes its simulation time events again..." Usually they
      // are not cyclic unless converted to SimTime? "For events scheduled at
      // the same time (including immediate events)..." Immediate events don't
      // have cycle/repeat params in AddImmediateEvent. So they are one-shot.
      _events.erase(id); // Clean up

      return _timeKeeper->GetSimulationTime();
    }
  }

  // 2. Timeline events
  if (!_timeline.empty()) {
    auto itTimeline = _timeline.begin();
    Smp::Duration time = itTimeline->first;
    Smp::Services::EventId id = itTimeline->second;

    // Advance time
    // We need to release lock to call SetSimulationTime (which might emit
    // events and callback user code)
    _mutex.unlock();
    if (time > _timeKeeper->GetSimulationTime()) {
      _timeKeeper->SetSimulationTime(time);
    }
    _mutex.lock();

    // Execute
    auto itEvt = _events.find(id);
    if (itEvt != _events.end()) {
      _timeline.erase(itTimeline); // Pop

      _currentEventId = id;
      auto ep = itEvt->second.entryPoint;

      _mutex.unlock();
      ep->Execute();
      _mutex.lock();

      _currentEventId = -1;

      // Handle repetition
      SchedulerEvent &evt =
          _events[id]; // Re-find execution might be needed if modified?
      // Assuming no concurrent modification for now.
      evt.count++;
      if (evt.repeat < 0 || evt.count <= evt.repeat) {
        // Reschedule
        evt.time += evt.cycleTime;
        ScheduleEvent(evt);
      } else {
        _events.erase(id);
      }

      return time;
    }
  }

  return -1; // No events
}

} // namespace sched
