#pragma once

#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ITimeKeeper.h>
#include <mutex>

namespace utils {

class TimeKeeper : public Smp::Services::ITimeKeeper {
public:
  TimeKeeper();
  virtual ~TimeKeeper() noexcept = default;

  void SetEventManager(Smp::Services::IEventManager *eventManager);

  Smp::Duration GetSimulationTime() const override;
  Smp::DateTime GetEpochTime() const override;
  Smp::DateTime GetMissionStartTime() const override;
  Smp::Duration GetMissionTime() const override;
  Smp::DateTime GetZuluTime() const override;

  void SetSimulationTime(Smp::Duration simulationTime) override;
  void SetEpochTime(Smp::DateTime epochTime) override;
  void SetMissionStartTime(Smp::DateTime missionStart) override;
  void SetMissionTime(Smp::Duration missionTime) override;

private:
  Smp::Services::IEventManager *_eventManager = nullptr;

  Smp::Duration _simulationTime = 0;
  Smp::DateTime _epochOffset = 0;
  Smp::DateTime _missionStart = 0;

  mutable std::mutex _mutex;
};

} // namespace utils
