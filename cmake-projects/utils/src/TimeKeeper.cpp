/**
 * @file TimeKeeper.cpp
 * @brief Implementation of the `utils::TimeKeeper` SMP service methods.
 *
 * @details Defines thread-safe getters and setters for simulation, epoch, mission,
 * and Zulu wall-clock time, plus post-lock time-change event notifications.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.44, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6
 * @compliance Implements `TimeKeeper` methods using `std::timed_mutex` RAII guards,
 * explicit `std::chrono` types (no `auto`), scoped lock release prior to event emission,
 * and control-flow comments.
 */

// Header under implementation and standard wall-clock time headers.
#include "utils/TimeKeeper.hpp"
#include <chrono>
#include <ctime>

namespace utils {

// Constructor for TimeKeeper: initializes base core::Object metadata.
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
  // Use RAII for mutex [CS-0010.22] with 1-second bounded wait [CS-0010.26].
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Fail fast if timed lock acquisition expired under contention.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring TimeKeeper lock");
  }
  _eventManager = eventManager;
}

Smp::Duration TimeKeeper::GetSimulationTime() const {
  // Implements FE-0070.2.6: GetSimulationTime shall return Simulation time.
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout error if _mutex cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring TimeKeeper lock");
  }
  return _simulationTime;
}

Smp::DateTime TimeKeeper::GetEpochTime() const {
  // Implements FE-0070.2.7: ITimeKeeper GetEpochTime shall return Epoch time.
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout error if _mutex cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring TimeKeeper lock");
  }
  return _epochOffset + _simulationTime;
}

Smp::DateTime TimeKeeper::GetMissionStartTime() const {
  // Implements FE-0070.2.9: ITimeKeeper GetMissionStartTime shall return Mission Start Time.
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout error if _mutex cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring TimeKeeper lock");
  }
  return _missionStart;
}

Smp::Duration TimeKeeper::GetMissionTime() const {
  // Implements FE-0070.2.8: ITimeKeeper GetMissionTime shall return Mission time.
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout error if _mutex cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring TimeKeeper lock");
  }
  return (GetEpochTime() - _missionStart);
}

Smp::DateTime TimeKeeper::GetZuluTime() const {
  // Implements FE-0070.2.10: ITimeKeeper GetZuluTime shall return Zulu time.
  // Get current system time in nanoseconds
  // [CS-0010.35] Forbidden auto replaced with explicit type
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::chrono::system_clock::duration duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

void TimeKeeper::SetSimulationTime(Smp::Duration simulationTime) {
  // Implements FE-0070.2.5: ITimeKeeper SetSimulationTime method shall advance Simulation time.
  // Use RAII for mutex [CS-0010.22] with 1-second bounded wait.
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Throw timeout error if _mutex cannot be acquired within 1 second.
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring TimeKeeper lock");
  }
  _simulationTime = simulationTime;
}

void TimeKeeper::SetEpochTime(Smp::DateTime epochTime) {
  // Implements FE-0070.2.2: The ITimeKeeper SetEpochTime method shall set the Epoch Time.
  // Set the new epoch time and calculate the offset inside a bounded lock scope.
  {
    std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
    // Abort update cleanly if timed lock acquisition fails.
    if (!lock.owns_lock()) {
      throw std::runtime_error("Timeout acquiring TimeKeeper lock");
    }
    _epochOffset = epochTime - _simulationTime;
  }
  
  // Emit event if manager is connected. This supports [FE-0070.3.21] which states
  // that EpochTimeChange events update scheduled epoch events.
  // Guard against detached event manager before emitting notification outside lock.
  if (_eventManager) {
    _eventManager->Emit(Smp::Services::IEventManager::SMP_EpochTimeChangedId);
  }
}

void TimeKeeper::SetMissionStartTime(Smp::DateTime missionStart) {
  // Implements FE-0070.2.3: The ITimeKeeper SetMissionStartTime method shall set the Mission start time.
  // Set the mission start time inside a bounded lock scope.
  {
    std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
    // Abort update cleanly if timed lock acquisition fails.
    if (!lock.owns_lock()) {
      throw std::runtime_error("Timeout acquiring TimeKeeper lock");
    }
    _missionStart = missionStart;
  }
  
  // Emit event if manager is connected. This supports [FE-0070.3.22] which states
  // that MissionTimeChange events update scheduled mission events.
  // Guard against detached event manager before emitting notification outside lock.
  if (_eventManager) {
    _eventManager->Emit(Smp::Services::IEventManager::SMP_MissionTimeChangedId);
  }
}

void TimeKeeper::SetMissionTime(Smp::Duration missionTime) {
  // Implements FE-0070.2.4: The ITimeKeeper SetMissionTime method shall set the Mission time.
  // MissionTime = EpochTime - MissionStart
  // => MissionStart = EpochTime - MissionTime
  SetMissionStartTime(GetEpochTime() - missionTime);
}

} // namespace utils
