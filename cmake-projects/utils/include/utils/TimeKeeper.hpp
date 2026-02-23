#pragma once

#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ITimeKeeper.h>
#include <core/Object.hpp>
#include <mutex>

namespace utils {

/**
 * @brief Implementation of the SMP Time Keeper Service.
 * @details Manages simulation, epoch, mission, and zulu time.
 */
class TimeKeeper : public core::Object,
                   public virtual Smp::Services::ITimeKeeper {
public:
  /**
   * @brief Default constructor.
   */
  TimeKeeper();

  /**
   * @brief Virtual destructor.
   */
  virtual ~TimeKeeper() noexcept = default;

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

  // ITimeKeeper methods

  /**
   * @brief Set the event manager to use for time change events.
   * @param eventManager The event manager service.
   */
  void SetEventManager(Smp::Services::IEventManager *eventManager);

  /**
   * @brief Get current simulation time.
   * @return Duration since start of simulation.
   */
  Smp::Duration GetSimulationTime() const override;

  /**
   * @brief Get current epoch time.
   * @return Absolute date and time.
   */
  Smp::DateTime GetEpochTime() const override;

  /**
   * @brief Get mission start time.
   * @return Absolute date and time of mission start.
   */
  Smp::DateTime GetMissionStartTime() const override;

  /**
   * @brief Get current mission time.
   * @return Duration since mission start.
   */
  Smp::Duration GetMissionTime() const override;

  /**
   * @brief Get current Zulu (wall clock) time.
   * @return Absolute date and time.
   */
  Smp::DateTime GetZuluTime() const override;

  /**
   * @brief Set simulation time.
   * @param simulationTime New simulation time.
   */
  void SetSimulationTime(Smp::Duration simulationTime) override;

  /**
   * @brief Set epoch time.
   * @param epochTime New epoch time.
   */
  void SetEpochTime(Smp::DateTime epochTime) override;

  /**
   * @brief Set mission start time.
   * @param missionStart New mission start time.
   */
  void SetMissionStartTime(Smp::DateTime missionStart) override;

  /**
   * @brief Set mission time.
   * @param missionTime New mission time.
   */
  void SetMissionTime(Smp::Duration missionTime) override;

private:
  /** @brief Pointer to the Event Manager service. */
  Smp::Services::IEventManager *_eventManager = nullptr;

  /** @brief Current simulation time. */
  Smp::Duration _simulationTime = 0;
  
  /** @brief Offset between simulation and epoch time. */
  Smp::DateTime _epochOffset = 0;
  
  /** @brief Absolute mission start time. */
  Smp::DateTime _missionStart = 0;

  /** @brief Mutex for thread-safe access. */
  mutable std::mutex _mutex;
};

} // namespace utils
