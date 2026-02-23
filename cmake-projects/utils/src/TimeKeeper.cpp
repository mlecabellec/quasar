#include "utils/TimeKeeper.hpp"
#include <chrono>
#include <ctime>

namespace utils {

/**
 * @brief Constructor for TimeKeeper.
 */
TimeKeeper::TimeKeeper()
    : core::Object("TimeKeeper", "SMP Time Keeper Service", nullptr) {}

Smp::ComponentStateKind TimeKeeper::GetState() const {
  return Smp::ComponentStateKind::CSK_Connected;
}

void TimeKeeper::Publish(Smp::IPublication *receiver) {
}

void TimeKeeper::Configure(Smp::Services::ILogger *logger,
                           Smp::Services::ILinkRegistry *linkRegistry) {
}

void TimeKeeper::Connect(Smp::ISimulator *simulator) {
}

void TimeKeeper::Disconnect() {
}

const Smp::Uuid &TimeKeeper::GetUuid() const {
  static Smp::Uuid uuid = {0, 0, 0, 0, 5}; // Generic Service UUID
  return uuid;
}

Smp::IField *TimeKeeper::GetField(Smp::String8 fullName) const {
  return nullptr;
}

const Smp::FieldCollection *TimeKeeper::GetFields() const { return nullptr; }

Smp::AnySimple TimeKeeper::GetSimpleValue(Smp::String8 fullName) const {
  return Smp::AnySimple();
}

void TimeKeeper::SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) {}

void TimeKeeper::GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                     Smp::AnySimple *values,
                                     Smp::UInt64 startIndex) const {}

void TimeKeeper::SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                     Smp::AnySimpleArray values,
                                     Smp::UInt64 startIndex) {}

Smp::Bool TimeKeeper::AddChild(Smp::IObject *child,
                               const Smp::ICollectionBase *collection) {
  return false;
}

Smp::Bool TimeKeeper::RemoveChild(Smp::IObject *child,
                                  const Smp::ICollectionBase *collection) {
  return false;
}

Smp::IObject *
TimeKeeper::IsChildInCollection(Smp::String8 child,
                                const Smp::ICollectionBase *collection) const {
  return nullptr;
}

Smp::IObject *TimeKeeper::GetChild(Smp::String8 name) const { return nullptr; }

void TimeKeeper::SetEventManager(Smp::Services::IEventManager *eventManager) {
  // Use RAII for mutex [CS-0010.22]
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
  // [CS-0010.35] Forbidden auto replaced with explicit type
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::chrono::system_clock::duration duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

void TimeKeeper::SetSimulationTime(Smp::Duration simulationTime) {
  // Use RAII for mutex [CS-0010.22]
  std::lock_guard<std::mutex> lock(_mutex);
  _simulationTime = simulationTime;
}

void TimeKeeper::SetEpochTime(Smp::DateTime epochTime) {
  // Set the new epoch time and calculate the offset
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _epochOffset = epochTime - _simulationTime;
  }
  
  // Emit event if manager is connected
  if (_eventManager) {
    _eventManager->Emit(Smp::Services::IEventManager::SMP_EpochTimeChangedId);
  }
}

void TimeKeeper::SetMissionStartTime(Smp::DateTime missionStart) {
  // Set the mission start time
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _missionStart = missionStart;
  }
  
  // Emit event if manager is connected
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
