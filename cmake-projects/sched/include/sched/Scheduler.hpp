#pragma once

#include <Smp/IEntryPoint.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <atomic>
#include <map>
#include <mutex>
#include <vector>

#include <core/Object.hpp>

namespace sched {

struct SchedulerEvent {
  Smp::Services::EventId id;
  const Smp::IEntryPoint *entryPoint;
  Smp::Duration time; // Absolute simulation time or dispatch time
  Smp::Duration cycleTime;
  Smp::Int64 repeat;
  Smp::Int64 count;      // Number of times executed
  Smp::Int64 sequenceId; // For stable ordering
  bool isMissionTime;
  bool isEpochTime;
  bool isZuluTime;
  bool isRelativeZulu;
  Smp::DateTime epochTime;   // For Epoch events
  Smp::Duration missionTime; // For Mission events
  Smp::DateTime zuluTime;    // For Zulu events
  Smp::Duration zuluDelay;   // For relative Zulu
};

/**
 * @brief Scheduler Service implementation.
 * @details Contributes to [FE-0070.3.1] (IScheduler interface).
 */
class Scheduler : public core::Object,
                  public virtual Smp::Services::IScheduler {
public:
  Scheduler(Smp::Services::ITimeKeeper *timeKeeper,
            Smp::Services::ILogger *logger);
  virtual ~Scheduler() noexcept = default;

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

  // IScheduler methods
  /// [FE-0070.3.9] Add an immediate Event.
  Smp::Services::EventId
  AddImmediateEvent(const Smp::IEntryPoint *entryPoint) override;
  /// [FE-0070.3.5] Add an Event at a specific simulation time.
  /// Contributes to [FE-0070.3.2] (repeat count) and [FE-0070.3.3] (cycle
  /// time).
  Smp::Services::EventId AddSimulationTimeEvent(
      const Smp::IEntryPoint *entryPoint, Smp::Duration simulationTime,
      Smp::Duration cycleTime = 0, Smp::Int64 repeat = 0) override;
  /// [FE-0070.3.6] Add an Event at a specific mission time.
  Smp::Services::EventId AddMissionTimeEvent(const Smp::IEntryPoint *entryPoint,
                                             Smp::Duration missionTime,
                                             Smp::Duration cycleTime = 0,
                                             Smp::Int64 repeat = 0) override;
  /// [FE-0070.3.7] Add an Event at a specific epoch time.
  Smp::Services::EventId AddEpochTimeEvent(const Smp::IEntryPoint *entryPoint,
                                           Smp::DateTime epochTime,
                                           Smp::Duration cycleTime = 0,
                                           Smp::Int64 repeat = 0) override;
  /// [FE-0070.3.8] Add an Event at a specific Zulu time.
  Smp::Services::EventId AddZuluTimeEvent(const Smp::IEntryPoint *entryPoint,
                                          Smp::DateTime zuluTime,
                                          Smp::Duration cycleTime = 0,
                                          Smp::Int64 repeat = 0) override;
  /// [FE-0070.3.XX] Add an Event at a relative Zulu time.
  Smp::Services::EventId AddRelativeZuluTimeEvent(
      const Smp::IEntryPoint *entryPoint, Smp::Duration deltaZuluTime,
      Smp::Duration cycleTime = 0, Smp::Int64 repeat = 0) override;

  /// [FE-0070.3.12] Update Event simulation time.
  void SetEventSimulationTime(Smp::Services::EventId event,
                              Smp::Duration simulationTime) override;
  /// [FE-0070.3.13] Update Event mission time.
  void SetEventMissionTime(Smp::Services::EventId event,
                           Smp::Duration missionTime) override;
  /// [FE-0070.3.14] Update Event epoch time.
  void SetEventEpochTime(Smp::Services::EventId event,
                         Smp::DateTime epochTime) override;
  /// [FE-0070.3.15] Update Event Zulu time.
  void SetEventZuluTime(Smp::Services::EventId event,
                        Smp::DateTime zuluTime) override;
  /// [FE-0070.3.16] Update Event cycle time.
  void SetEventCycleTime(Smp::Services::EventId event,
                         Smp::Duration cycleTime) override;
  /// [FE-0070.3.17] Update Event repeat count.
  void SetEventRepeat(Smp::Services::EventId event, Smp::Int64 repeat) override;
  /// [FE-0070.3.11] Remove an Event.
  void RemoveEvent(Smp::Services::EventId event) override;
  /// [FE-0070.3.YY] Check if an Event is scheduled.
  Smp::Bool IsEventScheduled(Smp::Services::EventId event) const override;
  /// [FE-0070.3.18] Return current EventId.
  Smp::Services::EventId GetCurrentEventId() const override;
  /// [FE-0070.3.19] Return next Event time.
  Smp::Duration GetNextScheduledEventTime() const override;

  // Custom methods for Simulator
  Smp::Duration ExecuteNextEvent(); // Returns next event time or -1 if none
  bool HasEvents() const;

private:
  Smp::Services::ITimeKeeper *_timeKeeper;
  Smp::Services::ILogger *_logger;

  mutable std::mutex _mutex;
  Smp::Services::EventId _nextEventId = 1;
  Smp::Int64 _nextSequenceId = 0;

  std::map<Smp::Services::EventId, SchedulerEvent> _events;

  // Ordered schedule. Key is (Time, Sequence).
  // Note: This map only stores Simulation Time events (including converted
  // Mission/Epoch). Immediate events are separate. Zulu events might need
  // separate handling or mapping to SimTime if possible (but Zulu is wall
  // clock...) Spec says: "The complete state of the Scheduler, with the
  // exception of Events scheduled using ZuluTime, shall be part of persisted
  // data" This implies Zulu time events are special. But for execution, we need
  // to check them. For now, let's treat Zulu events as "check every cycle" or
  // "separate thread"? "Events scheduled in Zulu Time are not considered, as
  // these Events do not have a fixed defined Simulation Time."

  // We will use a multimap for execution order.
  // Key: Simulation Time. Value: EventId.
  std::multimap<Smp::Duration, Smp::Services::EventId> _timeline;

  std::vector<Smp::Services::EventId> _immediateEvents;

  Smp::Services::EventId _currentEventId = -1;

  Smp::Services::EventId CreateEventId();
  void ScheduleEvent(const SchedulerEvent &evt);
  void RemoveFromTimeline(Smp::Services::EventId id);
};

} // namespace sched
