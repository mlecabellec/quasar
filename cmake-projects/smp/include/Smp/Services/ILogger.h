#ifndef SMP_SERVICES_ILOGGER_H
#define SMP_SERVICES_ILOGGER_H

#include "Smp/IService.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Services/LogMessageKind.h"

namespace Smp {
class IObject;

namespace Services {
class ILogger : public virtual IService {
public:
  static constexpr LogMessageKind LMK_Information = 0;
  static constexpr LogMessageKind LMK_Event = 1;
  static constexpr LogMessageKind LMK_Warning = 2;
  static constexpr LogMessageKind LMK_Error = 3;
  static constexpr LogMessageKind LMK_Debug = 4;

  static constexpr Char8 LMK_InformationName[] = "Information";
  static constexpr Char8 LMK_DebugName[] = "Debug";
  static constexpr Char8 LMK_ErrorName[] = "Error";
  static constexpr Char8 LMK_WarningName[] = "Warning";
  static constexpr Char8 LMK_EventName[] = "Event";

  static constexpr Char8 SMP_Logger[] = "Logger";

  virtual ~ILogger() noexcept = default;

  virtual LogMessageKind QueryLogMessageKind(String8 messageKindName) = 0;
  virtual void Log(const IObject *sender, String8 message,
                   LogMessageKind kind = 0) = 0;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_ILOGGER_H
