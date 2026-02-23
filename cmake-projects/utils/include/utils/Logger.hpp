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
