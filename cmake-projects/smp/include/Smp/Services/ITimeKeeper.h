#ifndef SMP_SERVICES_ITIMEKEEPER_H
#define SMP_SERVICES_ITIMEKEEPER_H

#include "Smp/IService.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Services/InvalidSimulationTime.h"

namespace Smp {
namespace Services {
class ITimeKeeper : public virtual IService {
public:
  static constexpr Char8 SMP_TimeKeeper[] = "TimeKeeper";

  virtual ~ITimeKeeper() noexcept = default;

  virtual Duration GetSimulationTime() const = 0;
  virtual DateTime GetEpochTime() const = 0;
  virtual DateTime GetMissionStartTime() const = 0;
  virtual Duration GetMissionTime() const = 0;
  virtual DateTime GetZuluTime() const = 0;

  virtual void SetSimulationTime(Duration simulationTime) = 0;
  virtual void SetEpochTime(DateTime epochTime) = 0;
  virtual void SetMissionStartTime(DateTime missionStart) = 0;
  virtual void SetMissionTime(Duration missionTime) = 0;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_ITIMEKEEPER_H
