#include "Smp/Services/IEventManager.h"
#include "Smp/Services/ILinkRegistry.h"
#include "Smp/Services/ILogger.h"
#include "Smp/Services/IResolver.h"
#include "Smp/Services/IScheduler.h"
#include "Smp/Services/ITimeKeeper.h"

namespace Smp {
namespace Services {

constexpr Smp::Char8 IEventManager::SMP_EventManager[];
constexpr Smp::Char8 ILinkRegistry::SMP_LinkRegistry[];
constexpr Smp::Char8 ILogger::SMP_Logger[];
constexpr Smp::Char8 IResolver::SMP_Resolver[];
constexpr Smp::Char8 IScheduler::SMP_Scheduler[];
constexpr Smp::Char8 ITimeKeeper::SMP_TimeKeeper[];

constexpr Smp::Char8 IEventManager::SMP_LeaveConnecting[];
constexpr Smp::Char8 IEventManager::SMP_EnterInitialising[];
constexpr Smp::Char8 IEventManager::SMP_LeaveInitialising[];
constexpr Smp::Char8 IEventManager::SMP_EnterStandby[];
constexpr Smp::Char8 IEventManager::SMP_LeaveStandby[];
constexpr Smp::Char8 IEventManager::SMP_EnterExecuting[];
constexpr Smp::Char8 IEventManager::SMP_LeaveExecuting[];
constexpr Smp::Char8 IEventManager::SMP_EnterStoring[];
constexpr Smp::Char8 IEventManager::SMP_LeaveStoring[];
constexpr Smp::Char8 IEventManager::SMP_EnterRestoring[];
constexpr Smp::Char8 IEventManager::SMP_LeaveRestoring[];
constexpr Smp::Char8 IEventManager::SMP_EnterExiting[];
constexpr Smp::Char8 IEventManager::SMP_EnterAborting[];
constexpr Smp::Char8 IEventManager::SMP_EpochTimeChanged[];
constexpr Smp::Char8 IEventManager::SMP_MissionTimeChanged[];
constexpr Smp::Char8 IEventManager::SMP_EnterReconnecting[];
constexpr Smp::Char8 IEventManager::SMP_LeaveReconnecting[];
constexpr Smp::Char8 IEventManager::SMP_PreSimTimeChange[];
constexpr Smp::Char8 IEventManager::SMP_PostSimTimeChange[];

constexpr Smp::Services::EventId IEventManager::SMP_LeaveConnectingId;
constexpr Smp::Services::EventId IEventManager::SMP_EnterInitialisingId;
constexpr Smp::Services::EventId IEventManager::SMP_LeaveInitialisingId;
constexpr Smp::Services::EventId IEventManager::SMP_EnterStandbyId;
constexpr Smp::Services::EventId IEventManager::SMP_LeaveStandbyId;
constexpr Smp::Services::EventId IEventManager::SMP_EnterExecutingId;
constexpr Smp::Services::EventId IEventManager::SMP_LeaveExecutingId;
constexpr Smp::Services::EventId IEventManager::SMP_EnterStoringId;
constexpr Smp::Services::EventId IEventManager::SMP_LeaveStoringId;
constexpr Smp::Services::EventId IEventManager::SMP_EnterRestoringId;
constexpr Smp::Services::EventId IEventManager::SMP_LeaveRestoringId;
constexpr Smp::Services::EventId IEventManager::SMP_EnterExitingId;
constexpr Smp::Services::EventId IEventManager::SMP_EnterAbortingId;
constexpr Smp::Services::EventId IEventManager::SMP_EpochTimeChangedId;
constexpr Smp::Services::EventId IEventManager::SMP_MissionTimeChangedId;
constexpr Smp::Services::EventId IEventManager::SMP_EnterReconnectingId;
constexpr Smp::Services::EventId IEventManager::SMP_LeaveReconnectingId;
constexpr Smp::Services::EventId IEventManager::SMP_PreSimTimeChangeId;
constexpr Smp::Services::EventId IEventManager::SMP_PostSimTimeChangeId;

constexpr Smp::Services::LogMessageKind ILogger::LMK_Information;
constexpr Smp::Services::LogMessageKind ILogger::LMK_Event;
constexpr Smp::Services::LogMessageKind ILogger::LMK_Warning;
constexpr Smp::Services::LogMessageKind ILogger::LMK_Error;
constexpr Smp::Services::LogMessageKind ILogger::LMK_Debug;
constexpr Smp::Char8 ILogger::LMK_InformationName[];
constexpr Smp::Char8 ILogger::LMK_DebugName[];
constexpr Smp::Char8 ILogger::LMK_ErrorName[];
constexpr Smp::Char8 ILogger::LMK_WarningName[];
constexpr Smp::Char8 ILogger::LMK_EventName[];

} // namespace Services
} // namespace Smp
