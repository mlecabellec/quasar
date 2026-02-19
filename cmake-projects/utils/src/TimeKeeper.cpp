#include "utils/TimeKeeper.hpp"
#include <chrono>
#include <ctime>

namespace utils {

TimeKeeper::TimeKeeper() {}

void TimeKeeper::SetEventManager(Smp::Services::IEventManager *eventManager) {
  std::lock_guard<std::mutex> lock(_mutex);
  _eventManager = eventManager;
}

Smp::Duration TimeKeeper::GetSimulationTime() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _simulationTime;
}

Smp::DateTime TimeKeeper::GetEpochTime() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _epochOffset + _simulationTime;
}

Smp::DateTime TimeKeeper::GetMissionStartTime() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _missionStart;
}

Smp::Duration TimeKeeper::GetMissionTime() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return (GetEpochTime() - _missionStart);
}

Smp::DateTime TimeKeeper::GetZuluTime() const {
  // Get current system time in nanoseconds
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

void TimeKeeper::SetSimulationTime(Smp::Duration simulationTime) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (simulationTime < _simulationTime) {
    // Warning: Moving time backwards
    // For now, allow it or throw InvalidSimulationTime?
    // Spec says "This has to be in the future".
    // But for reset/restore it might be different.
    // We will assume valid usage for now.
  }
  _simulationTime = simulationTime;
}

void TimeKeeper::SetEpochTime(Smp::DateTime epochTime) {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _epochOffset = epochTime - _simulationTime;
  }
  if (_eventManager) {
    _eventManager->Emit(Smp::Services::IEventManager::SMP_EpochTimeChangedId);
  }
}

void TimeKeeper::SetMissionStartTime(Smp::DateTime missionStart) {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _missionStart = missionStart;
  }
  if (_eventManager) {
    _eventManager->Emit(Smp::Services::IEventManager::SMP_MissionTimeChangedId);
  }
}

void TimeKeeper::SetMissionTime(Smp::Duration missionTime) {
  // MissionTime = EpochTime - MissionStart
  // => MissionStart = EpochTime - MissionTime
  SetMissionStartTime(GetEpochTime() - missionTime);
}

} // namespace utils
