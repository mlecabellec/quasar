#include "utils/Logger.hpp"
#include <cstring>
#include <iostream>

namespace utils {

Logger::Logger() : core::Object("Logger", "SMP Logger Service", nullptr) {}

Smp::ComponentStateKind Logger::GetState() const {
  return Smp::ComponentStateKind::CSK_Connected;
}

void Logger::Publish(Smp::IPublication *receiver) {}

void Logger::Configure(Smp::Services::ILogger *logger,
                       Smp::Services::ILinkRegistry *linkRegistry) {}

void Logger::Connect(Smp::ISimulator *simulator) {}

void Logger::Disconnect() {}

const Smp::Uuid &Logger::GetUuid() const {
  static Smp::Uuid uuid = {0, 0, 0, 0, 3}; // Generic Service UUID
  return uuid;
}

Smp::IField *Logger::GetField(Smp::String8 fullName) const { return nullptr; }

const Smp::FieldCollection *Logger::GetFields() const { return nullptr; }

Smp::AnySimple Logger::GetSimpleValue(Smp::String8 fullName) const {
  return Smp::AnySimple();
}

void Logger::SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) {}

void Logger::GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                 Smp::AnySimple *values,
                                 Smp::UInt64 startIndex) const {}

void Logger::SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                 Smp::AnySimpleArray values,
                                 Smp::UInt64 startIndex) {}

Smp::Bool Logger::AddChild(Smp::IObject *child,
                           const Smp::ICollectionBase *collection) {
  return false;
}

Smp::Bool Logger::RemoveChild(Smp::IObject *child,
                              const Smp::ICollectionBase *collection) {
  return false;
}

Smp::IObject *
Logger::IsChildInCollection(Smp::String8 child,
                            const Smp::ICollectionBase *collection) const {
  return nullptr;
}

Smp::IObject *Logger::GetChild(Smp::String8 name) const { return nullptr; }

Smp::Services::LogMessageKind
Logger::QueryLogMessageKind(Smp::String8 messageKindName) {
  if (std::strcmp(messageKindName,
                  Smp::Services::ILogger::LMK_InformationName) == 0)
    return Smp::Services::ILogger::LMK_Information;
  if (std::strcmp(messageKindName, Smp::Services::ILogger::LMK_EventName) == 0)
    return Smp::Services::ILogger::LMK_Event;
  if (std::strcmp(messageKindName, Smp::Services::ILogger::LMK_WarningName) ==
      0)
    return Smp::Services::ILogger::LMK_Warning;
  if (std::strcmp(messageKindName, Smp::Services::ILogger::LMK_ErrorName) == 0)
    return Smp::Services::ILogger::LMK_Error;
  if (std::strcmp(messageKindName, Smp::Services::ILogger::LMK_DebugName) == 0)
    return Smp::Services::ILogger::LMK_Debug;
  return -1; // Unknown kind
}

void Logger::Log(const Smp::IObject *sender, Smp::String8 message,
                 Smp::Services::LogMessageKind kind) {
  std::lock_guard<std::mutex> lock(_mutex);

  std::string kindStr;
  switch (kind) {
  case Smp::Services::ILogger::LMK_Information:
    kindStr = "INFO";
    break;
  case Smp::Services::ILogger::LMK_Event:
    kindStr = "EVENT";
    break;
  case Smp::Services::ILogger::LMK_Warning:
    kindStr = "WARN";
    break;
  case Smp::Services::ILogger::LMK_Error:
    kindStr = "ERROR";
    break;
  case Smp::Services::ILogger::LMK_Debug:
    kindStr = "DEBUG";
    break;
  default:
    kindStr = "UNKNOWN";
    break;
  }

  std::cout << "[" << kindStr << "] ";
  if (sender) {
    std::cout << "[" << sender->GetName() << "] ";
  }
  std::cout << message << std::endl;
}

} // namespace utils
