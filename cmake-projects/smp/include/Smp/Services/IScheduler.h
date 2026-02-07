#ifndef SMP_SERVICES_ISCHEDULER_H
#define SMP_SERVICES_ISCHEDULER_H

#include "Smp/IEntryPoint.h"
#include "Smp/IService.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Services/EventId.h"
#include "Smp/Services/InvalidCycleTime.h"
#include "Smp/Services/InvalidEventId.h"
#include "Smp/Services/InvalidEventTime.h"

namespace Smp {
namespace Services {
class IScheduler : public virtual IService {
public:
  static constexpr Char8 SMP_Scheduler[] = "Scheduler";

  virtual ~IScheduler() noexcept = default;

  virtual EventId AddImmediateEvent(const IEntryPoint *entryPoint) = 0;
  virtual EventId AddSimulationTimeEvent(const IEntryPoint *entryPoint,
                                         Duration simulationTime,
                                         Duration cycleTime = 0,
                                         Int64 repeat = 0) = 0;
  virtual EventId AddMissionTimeEvent(const IEntryPoint *entryPoint,
                                      Duration missionTime,
                                      Duration cycleTime = 0,
                                      Int64 repeat = 0) = 0;
  virtual EventId AddEpochTimeEvent(const IEntryPoint *entryPoint,
                                    DateTime epochTime, Duration cycleTime = 0,
                                    Int64 repeat = 0) = 0;
  virtual EventId AddZuluTimeEvent(const IEntryPoint *entryPoint,
                                   DateTime zuluTime, Duration cycleTime = 0,
                                   Int64 repeat = 0) = 0;

  virtual void SetEventSimulationTime(EventId event,
                                      Duration simulationTime) = 0;
  virtual void SetEventMissionTime(EventId event, Duration missionTime) = 0;
  virtual void SetEventEpochTime(EventId event, DateTime epochTime) = 0;
  virtual void SetEventZuluTime(EventId event, DateTime zuluTime) = 0;
  virtual void SetEventCycleTime(EventId event, Duration cycleTime) = 0;
  virtual void SetEventRepeat(EventId event, Int64 repeat) = 0;

  virtual void RemoveEvent(EventId event) = 0;
  virtual EventId GetCurrentEventId() const = 0;
  virtual Duration GetNextScheduledEventTime() const = 0;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_ISCHEDULER_H
