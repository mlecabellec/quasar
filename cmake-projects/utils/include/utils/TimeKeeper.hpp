/**
 * @file TimeKeeper.hpp
 * @brief Implementation of the SMP Time Keeper Service.
 *
 * @details Manages simulation, epoch, mission, and zulu time.
 *
 * @note This utility class implements SMP services and is thread-safe.
 *       It does not directly contribute to the features described in FE-0020 (NamedObject, tree/graph structures).
 *       The thread safety implemented here is for the TimeKeeper's own operations and not related to FE-0020 requirements.
 *       For example, FE-0020.5 requires thread-safe operations on named objects, which are not implemented in this file.
 *
 * @par Contribution to FE-0080 (Catalogue to C++ Mapping & Type Mapping):
 * - This class is a C++ implementation of the SMP ITimeKeeper service, aligning with [FE-0080.6.13] (Service types shall be mapped to C++ classes).
 * - Declared within the `utils` namespace, adhering to [FE-0080.5.3] (Elements shall be declared within the exact same namespace as in the Catalogue).
 * - Defined in a dedicated header file (`TimeKeeper.hpp`), supporting [FE-0080.5.4] (Each type shall be declared in a dedicated header file), [FE-0080.5.5] (Header files shall allow multiple inclusion), and [FE-0080.5.6] (Header files shall avoid circular dependencies).
 * - Utilizes C++ standard features like `std::chrono` and `std::timed_mutex`, aligning with [FE-0080.4.1] (C++ standard version at least C++11).
 * - Member variables like `_simulationTime`, `_epochOffset`, `_missionStart` represent mapped fields or simple value elements, according to [FE-0080.5.9] and [FE-0080.5.13].
 * - Member methods like `GetSimulationTime`, `SetEpochTime` represent mapped operations, adhering to [FE-0080.5.20] (Operation elements shall be mapped to C++ member methods).
 * - The `GetUuid()` method returns an `Smp::Uuid`, contributing to [FE-0080.6.1] (UUID variable declaration).
 * - Visibility is managed using C++ access specifiers (`public`, `private`), mapping to [FE-0080.5.7] (Visibility kind attributes shall be mapped to ISO/ANSI C++ member access specifiers).
 * - **Contribution to FE-0090 (Datacodec):** This class does not directly contribute to FE-0090. Its primary function is time management and it is not involved in binary data serialization, deserialization, schema definitions, or codec implementations.
 *
 * @par Contribution to FE-0030:
 * - This class implements the SMP ITimeKeeper service and does not directly implement the `Number`, `String`, `Buffer`, or `BitBuffer` types or their named variants as described in FE-0030.1-7.
 * - It adheres to FE-0030.8 by utilizing `std::timed_mutex` for thread-safe operations.
 * - It adheres to FE-0030.9 by exhibiting `const` correctness in its methods.
 * - @warning Tests specifically proving contributions to FE-0030 are not present within this module, and documentation linking this module to FE-0030 is missing.
 *
 * @par Contribution to FE-0070.2 (Time Keeper):
 * - Implements the ITimeKeeper component [FE-0070.2.1].
 * - Provides all required Get methods: GetSimulationTime [FE-0070.2.6], GetEpochTime [FE-0070.2.7], GetMissionTime [FE-0070.2.8], GetMissionStartTime [FE-0070.2.9], and GetZuluTime [FE-0070.2.10].
 * - Provides all required Set methods: SetEpochTime [FE-0070.2.2], SetMissionStartTime [FE-0070.2.3], SetMissionTime [FE-0070.2.4], and SetSimulationTime [FE-0070.2.5].
 * - Supports time change event emissions, indirectly supporting scheduler updates [FE-0070.3.21, FE-0070.3.22].
 */
#pragma once

#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ITimeKeeper.h>
#include <core/Object.hpp>
#include <mutex>

namespace utils {

/**
 * @brief Implementation of the SMP Time Keeper Service.
 * @details Manages simulation, epoch, mission, and zulu time.
 *
 * @note This class is thread-safe using std::timed_mutex.
 *       It does not implement NamedObject features as defined in FE-0020. Specifically, it does not provide
 *       a NamedObject class, parent/child management, or tree/graph traversal utilities as described in FE-0020.1, FE-0020.4, FE-0020.12, etc.
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
  virtual ~TimeKeeper() noexcept override = default;

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
  mutable std::timed_mutex _mutex;
};

} // namespace utils
