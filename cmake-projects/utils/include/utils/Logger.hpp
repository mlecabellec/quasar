#pragma once

#include <Smp/IObject.h>
#include <Smp/Services/ILogger.h>
#include <core/Object.hpp>
#include <iostream>
#include <mutex>

namespace utils {

class Logger : public core::Object, public virtual Smp::Services::ILogger {
public:
  Logger();
  virtual ~Logger() noexcept = default;

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
                     const Smp::IObject *collection) override;
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::IObject *collection) override;
  Smp::IObject *
  IsChildInCollection(Smp::String8 child,
                      const Smp::IObject *collection) const override;

  Smp::IObject *GetChild(Smp::String8 name) const override;

  // ILogger methods

  Smp::Services::LogMessageKind
  QueryLogMessageKind(Smp::String8 messageKindName) override;

  void Log(const Smp::IObject *sender, Smp::String8 message,
           Smp::Services::LogMessageKind kind = 0) override;

private:
  std::mutex _mutex;
};

} // namespace utils
