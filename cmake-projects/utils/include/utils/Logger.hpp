/**
 * @file Logger.hpp
 * @brief Implementation of the SMP Logger Service.
 *
 * @details Provides message logging capabilities to stdout.
 *
 * @note This utility class implements SMP services and is thread-safe.
 *       It does not directly contribute to the features described in FE-0020 (NamedObject, tree/graph structures).
 *       The thread safety implemented here is for the Logger's own operations and not related to FE-0020 requirements.
 *       For example, FE-0020.5 requires thread-safe operations on named objects, which are not implemented in this file.
 *
 * @par Contribution to FE-0080 (Catalogue to C++ Mapping & Type Mapping):
 * - This class is a C++ implementation of the SMP ILogger service, aligning with [FE-0080.6.13] (Service types shall be mapped to C++ classes).
 * - Declared within the `utils` namespace, adhering to [FE-0080.5.3] (Elements shall be declared within the exact same namespace as in the Catalogue).
 * - Defined in a dedicated header file (`Logger.hpp`), supporting [FE-0080.5.4] (Each type shall be declared in a dedicated header file), [FE-0080.5.5] (Header files shall allow multiple inclusion), and [FE-0080.5.6] (Header files shall avoid circular dependencies).
 * - Utilizes C++ standard features like `std::timed_mutex`, aligning with [FE-0080.4.1] (C++ standard version at least C++11).
 * - Member variables like `_mutex` represent mapped fields or simple value elements, according to [FE-0080.5.9] and [FE-0080.5.13].
 * - Member methods like `QueryLogMessageKind`, `Log` represent mapped operations, adhering to [FE-0080.5.20] (Operation elements shall be mapped to C++ member methods).
 * - The `GetUuid()` method returns an `Smp::Uuid`, contributing to [FE-0080.6.1] (UUID variable declaration).
 * - Visibility is managed using C++ access specifiers (`public`, `private`), mapping to [FE-0080.5.7] (Visibility kind attributes shall be mapped to ISO/ANSI C++ member access specifiers).
 * - **Contribution to FE-0090 (Datacodec):** This class does not directly contribute to FE-0090. Its purpose is message logging and it is not involved in binary data serialization, deserialization, schema definitions, or codec implementations.
 *
 * @par Contribution to FE-0030:
 * - This class implements the SMP ILogger service and does not directly implement the `Number`, `String`, `Buffer`, or `BitBuffer` types or their named variants as described in FE-0030.1-7.
 * - It adheres to FE-0030.8 by utilizing `std::timed_mutex` for thread-safe operations.
 * - It adheres to FE-0030.9 by exhibiting `const` correctness in its methods.
 * - @warning Tests specifically proving contributions to FE-0030 are not present within this module, and documentation linking this module to FE-0030 is missing.
 *
 * @par Contribution to FE-0070.1 (Logger):
 * - Implements the ILogger component [FE-0070.1.1].
 * - Provides `QueryLogMessageKind` which translates log message kind names to IDs [FE-0070.1.3].
 * - The `Log` method implements message logging to stdout [FE-0070.1.6].
 * - Supports default Log Message Kinds (Debug, Error, Warning, Event, Information) as defined in Table 5-7 [FE-0070.1.4].
 * - @warning The mapping of Log Message Kinds is hardcoded and not persisted as required by [FE-0070.1.5].
 * - @warning The explicit maintenance of a mapping of Log Message Kinds as per [FE-0070.1.2] is handled via hardcoded logic in `QueryLogMessageKind`.
 */
#pragma once

#include <Smp/IObject.h>
#include <Smp/Services/ILogger.h>
#include <core/Object.hpp>
#include <iostream>
#include <mutex>

namespace utils {

/**
 * @brief Implementation of the SMP Logger Service.
 * @details Provides message logging capabilities to stdout.
 *
 * @note This class is thread-safe using std::timed_mutex.
 *       It does not implement NamedObject features as defined in FE-0020. Specifically, it does not provide
 *       a NamedObject class, parent/child management, or tree/graph traversal utilities as described in FE-0020.1, FE-0020.4, FE-0020.12, etc.
 */
class Logger : public core::Object, public virtual Smp::Services::ILogger {
public:
  /**
   * @brief Default constructor.
   */
  Logger();

  /**
   * @brief Virtual destructor.
   */
  virtual ~Logger() noexcept override = default;

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

  // ILogger methods

  /**
   * @brief Query the log message kind ID for a given name.
   * @param messageKindName Name of the message kind.
   * @return LogMessageKind associated with the name.
   */
  Smp::Services::LogMessageKind
  QueryLogMessageKind(Smp::String8 messageKindName) override;

  /**
   * @brief Log a message.
   * @param sender The object sending the message.
   * @param message The message text.
   * @param kind The kind of message.
   */
  void Log(const Smp::IObject *sender, Smp::String8 message,
           Smp::Services::LogMessageKind kind = 0) override;

private:
  /** @brief Mutex for thread-safe logging. */
  std::timed_mutex _mutex;
};

} // namespace utils
