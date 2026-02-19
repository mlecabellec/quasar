#include "utils/Logger.hpp"
#include <cstring>
#include <iostream>

namespace utils {

Logger::Logger() {}

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
