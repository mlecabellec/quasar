/**
 * @file Logger.cpp
 * @brief Implementation of the `utils::Logger` SMP service methods.
 *
 * @details Defines predefined SMP log message kind lookup and timed-mutex
 * serialized standard output formatting for `utils::Logger`.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.44, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6
 * @compliance Implements `Logger` methods with null-pointer guards, timed mutex
 * serialization, exhaustive switch default handling, and control-flow comments.
 */

// Header under implementation and standard C-string/iostream headers.
#include "utils/Logger.hpp"
#include <cstring>
#include <iostream>

namespace utils {

// Constructor for Logger: initializes base core::Object metadata.
Logger::Logger() : core::Object("Logger", "SMP Logger Service", nullptr) {}

Smp::ComponentStateKind Logger::GetState() const {
  return Smp::ComponentStateKind::CSK_Connected;
}

void Logger::Publish(Smp::IPublication *receiver) {
  // [CS-0010.16] Check for null pointers before any future publication access.
  if (!receiver) {
    return;
  }
}

void Logger::Configure(Smp::Services::ILogger *logger,
                       Smp::Services::ILinkRegistry *linkRegistry) {
}

void Logger::Connect(Smp::ISimulator *simulator) {
}

void Logger::Disconnect() {
}

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
  // [CS-0010.16] Guard against null message kind names to prevent std::strcmp UB.
  if (!messageKindName) {
    return -1;
  }

  // Implements FE-0070.1.3: Translates log message kind names to IDs.
  // This function supports the predefined log message kinds as per FE-0070.1.4.
  // The mapping is hardcoded, not persisted, and does not dynamically maintain a map,
  // thus not fully satisfying FE-0070.1.2 or FE-0070.1.5.

  // Compare against SMP Information message kind name.
  if (std::strcmp(messageKindName,
                  Smp::Services::ILogger::LMK_InformationName) == 0)
    return Smp::Services::ILogger::LMK_Information;

  // Compare against SMP Event message kind name.
  if (std::strcmp(messageKindName, Smp::Services::ILogger::LMK_EventName) == 0)
    return Smp::Services::ILogger::LMK_Event;

  // Compare against SMP Warning message kind name.
  if (std::strcmp(messageKindName, Smp::Services::ILogger::LMK_WarningName) ==
      0)
    return Smp::Services::ILogger::LMK_Warning;

  // Compare against SMP Error message kind name.
  if (std::strcmp(messageKindName, Smp::Services::ILogger::LMK_ErrorName) == 0)
    return Smp::Services::ILogger::LMK_Error;

  // Compare against SMP Debug message kind name.
  if (std::strcmp(messageKindName, Smp::Services::ILogger::LMK_DebugName) == 0)
    return Smp::Services::ILogger::LMK_Debug;
  return -1; // Unknown kind
}

void Logger::Log(const Smp::IObject *sender, Smp::String8 message,
                 Smp::Services::LogMessageKind kind) {
  // [CS-0010.16] Return safely on null message pointers prior to locking.
  if (!message) {
    return;
  }

  // Use RAII for mutex [CS-0010.22] bounded to a 1-second timeout [CS-0010.26].
  std::unique_lock<std::timed_mutex> lock(_mutex, std::chrono::seconds(1));
  // Drop message cleanly if lock acquisition timed out under contention.
  if (!lock.owns_lock()) {
    // If we cannot acquire the lock, we skip logging to avoid blocking.
    return;
  }

  // Implements FE-0070.1.6: Logs a message to stdout.
  // Formats the output including the log kind (e.g., INFO, ERROR) and sender if available,
  // aligning with FE-0070.1.4.

  // Map numeric SMP log message kind to a human-readable severity tag.
  std::string kindStr;
  // Dispatch on standard SMP LogMessageKind identifiers with explicit default fallback.
  switch (kind) {
  // Format Information messages with INFO tag.
  case Smp::Services::ILogger::LMK_Information:
    kindStr = "INFO";
    break;
  // Format Event messages with EVENT tag.
  case Smp::Services::ILogger::LMK_Event:
    kindStr = "EVENT";
    break;
  // Format Warning messages with WARN tag.
  case Smp::Services::ILogger::LMK_Warning:
    kindStr = "WARN";
    break;
  // Format Error messages with ERROR tag.
  case Smp::Services::ILogger::LMK_Error:
    kindStr = "ERROR";
    break;
  // Format Debug messages with DEBUG tag.
  case Smp::Services::ILogger::LMK_Debug:
    kindStr = "DEBUG";
    break;
  // Handle unrecognized custom message kinds safely as UNKNOWN.
  default:
    kindStr = "UNKNOWN";
    break;
  }

  // Output the formatted log message prefix.
  std::cout << "[" << kindStr << "] ";
  // Include sender object name prefix only when a valid sender pointer is supplied.
  if (sender) {
    std::cout << "[" << sender->GetName() << "] ";
  }
  std::cout << message << std::endl;
}

} // namespace utils
