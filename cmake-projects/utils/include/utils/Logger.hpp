#pragma once

#include <Smp/IObject.h>
#include <Smp/Services/ILogger.h>
#include <iostream>
#include <mutex>

namespace utils {

class Logger : public Smp::Services::ILogger {
public:
  Logger();
  virtual ~Logger() noexcept = default;

  Smp::Services::LogMessageKind
  QueryLogMessageKind(Smp::String8 messageKindName) override;

  void Log(const Smp::IObject *sender, Smp::String8 message,
           Smp::Services::LogMessageKind kind = 0) override;

private:
  std::mutex _mutex;
};

} // namespace utils
