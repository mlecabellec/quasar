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

/**
 * @brief Representation of a scheduled event.
 */
struct SchedulerEvent {
  /** @brief Unique event identifier. */
  Smp::Services::EventId id = 0;
  
  /** @brief Entry point to execute. */
  const Smp::IEntryPoint *entryPoint = nullptr;
  
  /** @brief Absolute simulation time or dispatch time. */
  Smp::Duration time = 0;
  
  /** @brief Cycle time for recurring events. */
  Smp::Duration cycleTime = 0;
  
  /** @brief Total repeat count. */
  Smp::Int64 repeat = 0;
  
  /** @brief Current execution count. */
  Smp::Int64 count = 0;
  
  /** @brief Sequence ID for stable ordering of events at same time. */
  Smp::Int64 sequenceId = 0;
  
  /** @brief Whether event is based on mission time. */
  bool isMissionTime = false;
  
  /** @brief Whether event is based on epoch time. */
  bool isEpochTime = false;
  
  /** @brief Whether event is based on Zulu time. */
  bool isZuluTime = false;
  
  /** @brief Whether event is relative to current Zulu time. */
  bool isRelativeZulu = false;
  
  /** @brief Original epoch time requested. */
  Smp::DateTime epochTime = 0;
  
  /** @brief Original mission time requested. */
  Smp::Duration missionTime = 0;
  
  /** @brief Original Zulu time requested. */
  Smp::DateTime zuluTime = 0;
  
  /** @brief Original Zulu delay requested. */
  Smp::Duration zuluDelay = 0;
};

/**
 * @brief Implementation of the SMP Scheduler Service.
 * @details Contributes to [FE-0070.3.1].
 */
class Scheduler : public core::Object,
                  public virtual Smp::Services::IScheduler {
public:
  /**
   * @brief Constructor.
   * @param timeKeeper TimeKeeper service.
   * @param logger Logger service.
   */
  Scheduler(Smp::Services::ITimeKeeper *timeKeeper,
            Smp::Services::ILogger *logger);
            
  /**
   * @brief Virtual destructor.
   */
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
      const Smp::IEntryPoint *entryPoint, Smp::Duration deltaZuluTime,
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
  Smp::Bool IsEventScheduled(Smp::Services::EventId event) const override;
  Smp::Services::EventId GetCurrentEventId() const override;
  Smp::Duration GetNextScheduledEventTime() const override;

  /**
   * @brief Execute the next event in the timeline.
   * @return The time at which the event was executed, or -1 if no events.
   */
  Smp::Duration ExecuteNextEvent();
  
  /**
   * @brief Check if there are any events scheduled.
   * @return True if events exist.
   */
  bool HasEvents() const;

private:
  /** @brief Pointer to the TimeKeeper service. */
  Smp::Services::ITimeKeeper *_timeKeeper = nullptr;
  
  /** @brief Pointer to the Logger service. */
  Smp::Services::ILogger *_logger = nullptr;

  /** @brief Mutex for thread-safe access. */
  mutable std::mutex _mutex;
  
  /** @brief ID counter for new events. */
  Smp::Services::EventId _nextEventId = 1;
  
  /** @brief Sequence ID counter for stable ordering. */
  Smp::Int64 _nextSequenceId = 0;

  /** @brief Collection of all events by ID. */
  std::map<Smp::Services::EventId, SchedulerEvent> _events;

  /** @brief Ordered execution timeline. Map key is Simulation Time. */
  std::multimap<Smp::Duration, Smp::Services::EventId> _timeline;

  /** @brief List of immediate events to execute. */
  std::vector<Smp::Services::EventId> _immediateEvents;

  /** @brief ID of the event currently being executed. */
  Smp::Services::EventId _currentEventId = -1;

  /**
   * @brief Create a new unique event ID.
   * @return New event ID.
   */
  Smp::Services::EventId CreateEventId();
  
  /**
   * @brief Add an event to the execution timeline.
   * @param evt The event to schedule.
   */
  void ScheduleEvent(const SchedulerEvent &evt);
  
  /**
   * @brief Remove an event from the execution timeline.
   * @param id The event ID to remove.
   */
  void RemoveFromTimeline(Smp::Services::EventId id);
};

} // namespace sched
