/**
 * @file EventManager.hpp
 * @brief Implementation of the SMP Event Manager Service.
 *
 * @details This class provides mechanisms for event subscription, unsubscription, and emission.
 * Contributes to [FE-0070.4.1].
 *
 * @note This class is thread-safe using std::timed_mutex.
 *       It does not implement NamedObject features as defined in FE-0020. Specifically, it does not provide
 *       a NamedObject class, parent/child management, or tree/graph traversal utilities as described in FE-0020.1, FE-0020.4, FE-0020.12, etc.
 *
 * @par Contribution to FE-0080 (Catalogue to C++ Mapping & Type Mapping):
 * - This class is a C++ implementation of the SMP IEventManager service, aligning with [FE-0080.6.13] (Service types shall be mapped to C++ classes).
 * - Declared within the `utils` namespace, adhering to [FE-0080.5.3] (Elements shall be declared within the exact same namespace as in the Catalogue).
 * - Defined in a dedicated header file (`EventManager.hpp`), supporting [FE-0080.5.4] (Each type shall be declared in a dedicated header file), [FE-0080.5.5] (Header files shall allow multiple inclusion), and [FE-0080.5.6] (Header files shall avoid circular dependencies).
 * - Utilizes C++ standard features like `std::timed_mutex`, aligning with [FE-0080.4.1] (C++ standard version at least C++11).
 * - Member variables like `_nextEventId`, `_eventIds`, `_subscriptions` represent mapped fields or simple value elements, according to [FE-0080.5.9] and [FE-0080.5.13].
 * - Member methods like `QueryEventId`, `Subscribe`, `Emit` represent mapped operations, adhering to [FE-0080.5.20] (Operation elements shall be mapped to C++ member methods).
 * - The `GetUuid()` method returns an `Smp::Uuid`, contributing to [FE-0080.6.1] (UUID variable declaration).
 * - Visibility is managed using C++ access specifiers (`public`, `private`), mapping to [FE-0080.5.7] (Visibility kind attributes shall be mapped to ISO/ANSI C++ member access specifiers).
 * - **Contribution to FE-0090 (Datacodec):** This class does not directly contribute to FE-0090. It implements event management services and does not provide data types such as `BitBufferSlice`, `NamedObject` hierarchies, or codec implementations required for binary serialization and deserialization.
 *
 * @par Contribution to FE-0030:
 * - This class implements the SMP IEventManager service and does not directly implement the `Number`, `String`, `Buffer`, or `BitBuffer` types or their named variants as described in FE-0030.1-7.
 * - It adheres to FE-0030.8 by utilizing `std::timed_mutex` for thread-safe operations.
 * - It adheres to FE-0030.9 by exhibiting `const` correctness in its methods.
 * - @warning Tests specifically proving contributions to FE-0030 are not present within this module, and documentation linking this module to FE-0030 is missing.
 *
 * @par Contribution to FE-0070.4 (Event Manager):
 * - Implements the IEventManager component [FE-0070.4.1].
 * - Supports event identification via `QueryEventId` [FE-0070.4.2].
 * - Maintains internal lists for event IDs and entry points, fulfilling [FE-0070.4.3] and [FE-0070.4.4] (initially empty).
 * - Provides `Subscribe` [FE-0070.4.5] and `Unsubscribe` [FE-0070.4.6] functionality.
 * - Implements `Emit` for global event emission [FE-0070.4.7], supporting synchronous flag [FE-0070.4.9].
 * - Registers predefined global events, aligning with [FE-0070.4.8].
 * - @warning The implementation of [FE-0070.4.10] (State transition event shall not trigger another transition) is not explicitly handled in the Emit logic.
 * - The copying of subscribers during Emit helps address [FE-0070.4.11].
 */
#pragma once

#include <Smp/IObject.h>
#include <Smp/Services/IEventManager.h>
#include <core/Object.hpp>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace utils {

/**
 * @brief Implementation of the SMP Event Manager Service.
 * @details This class provides mechanisms for event subscription, unsubscription, and emission.
 * Contributes to [FE-0070.3.1].
 *
 * @note This class is thread-safe using std::timed_mutex.
 *       It does not implement NamedObject features as defined in FE-0020. Specifically, it does not provide
 *       a NamedObject class, parent/child management, or tree/graph traversal utilities as described in FE-0020.1, FE-0020.4, FE-0020.12, etc.
 */
class EventManager : public core::Object,
                     public virtual Smp::Services::IEventManager {
public:
  /**
   * @brief Default constructor.
   */
  EventManager();

  /**
   * @brief Virtual destructor.
   */
  virtual ~EventManager() noexcept override = default;

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

  // IEventManager methods

  /**
   * @brief Query the event ID for a given event name.
   * @param eventName Name of the event.
   * @return EventId associated with the name.
   */
  Smp::Services::EventId QueryEventId(Smp::String8 eventName) override;

  /**
   * @brief Subscribe an entry point to an event.
   * @param event The event ID.
   * @param entryPoint The entry point to subscribe.
   */
  void Subscribe(Smp::Services::EventId event,
                 const Smp::IEntryPoint *entryPoint) override;

  /**
   * @brief Unsubscribe an entry point from an event.
   * @param event The event ID.
   * @param entryPoint The entry point to unsubscribe.
   */
  void Unsubscribe(Smp::Services::EventId event,
                   const Smp::IEntryPoint *entryPoint) override;

  /**
   * @brief Emit an event.
   * @param event The event ID.
   * @param synchronous Whether the emission is synchronous.
   */
  void Emit(Smp::Services::EventId event,
            Smp::Bool synchronous = true) override;

private:
  /** @brief Next available event ID for dynamic creation. */
  Smp::Services::EventId _nextEventId = 20;
  
  /** @brief Mapping from event name to event ID. */
  std::map<std::string, Smp::Services::EventId> _eventIds;
  
  /** @brief Mapping from event ID to event name. */
  std::map<Smp::Services::EventId, std::string> _eventNames;
  
  /** @brief Subscription list for each event ID. */
  std::map<Smp::Services::EventId, std::vector<const Smp::IEntryPoint *>> _subscriptions;

  /** @brief Mutex for thread-safe access. */
  mutable std::timed_mutex _mutex;

  /**
   * @brief Register standard SMP predefined events.
   */
  void RegisterPredefinedEvents();
};

} // namespace utils
