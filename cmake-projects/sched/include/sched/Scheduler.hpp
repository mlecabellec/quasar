#pragma once

#include <Smp/IEntryPoint.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <atomic>
#include <map>
#include <mutex>
#include <vector>

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

class Scheduler : public Smp::Services::IScheduler {
public:
  Scheduler(Smp::Services::ITimeKeeper *timeKeeper,
            Smp::Services::ILogger *logger);
  virtual ~Scheduler() noexcept = default;

  // IScheduler methods
  Smp::Services::EventId
  AddImmediateEvent(const Smp::IEntryPoint *entryPoint) override;
  Smp::Services::EventId AddSimulationTimeEvent(
      const Smp::IEntryPoint *entryPoint, Smp::Duration simulationTime,
      Smp::Duration cycleTime = 0, Smp::Int64 repeat = 0) override;
  Smp::Services::EventId AddMissionTimeEvent(const Smp::IEntryPoint *entryPoint,
                                             Smp::Duration missionTime,
                                             Smp::Duration cycleTime = 0,
                                             Smp::Int64 repeat = 0) override;
  Smp::Services::EventId AddEpochTimeEvent(const Smp::IEntryPoint *entryPoint,
                                           Smp::DateTime epochTime,
                                           Smp::Duration cycleTime = 0,
                                           Smp::Int64 repeat = 0) override;
  Smp::Services::EventId AddZuluTimeEvent(const Smp::IEntryPoint *entryPoint,
                                          Smp::DateTime zuluTime,
                                          Smp::Duration cycleTime = 0,
                                          Smp::Int64 repeat = 0) override;
  Smp::Services::EventId AddRelativeZuluTimeEvent(
      const Smp::IEntryPoint *entryPoint, Smp::Duration zuluTimeDelay,
      Smp::Duration cycleTime = 0, Smp::Int64 repeat = 0) override;

  void SetEventSimulationTime(Smp::Services::EventId event,
                              Smp::Duration simulationTime) override;
  void SetEventMissionTime(Smp::Services::EventId event,
                           Smp::Duration missionTime) override;
  void SetEventEpochTime(Smp::Services::EventId event,
                         Smp::DateTime epochTime) override;
  void SetEventZuluTime(Smp::Services::EventId event,
                        Smp::DateTime zuluTime) override;
  void SetEventCycleTime(Smp::Services::EventId event,
                         Smp::Duration cycleTime) override;
  void SetEventRepeat(Smp::Services::EventId event, Smp::Int64 repeat) override;
  void RemoveEvent(Smp::Services::EventId event) override;
  Smp::Services::EventId GetCurrentEventId() const override;
  Smp::Duration GetNextScheduledEventTime() const override;
  Smp::Bool IsEventScheduled(Smp::Services::EventId eventId) const override;

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
