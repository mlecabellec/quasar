#ifndef SMP_SERVICES_IEVENTMANAGER_H
#define SMP_SERVICES_IEVENTMANAGER_H

#include "Smp/IEntryPoint.h"
#include "Smp/IService.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Services/EntryPointAlreadySubscribed.h"
#include "Smp/Services/EntryPointNotSubscribed.h"
#include "Smp/Services/EventId.h"
#include "Smp/Services/InvalidEventId.h"
#include "Smp/Services/InvalidEventName.h"

namespace Smp {
namespace Services {
class IEventManager : public virtual IService {
public:
  static constexpr Char8 SMP_EventManager[] = "EventManager";

  static constexpr EventId SMP_LeaveConnectingId = 1;
  static constexpr Char8 SMP_LeaveConnecting[] = "SMP_LeaveConnecting";

  static constexpr EventId SMP_EnterInitialisingId = 2;
  static constexpr Char8 SMP_EnterInitialising[] = "SMP_EnterInitialising";

  static constexpr EventId SMP_LeaveInitialisingId = 3;
  static constexpr Char8 SMP_LeaveInitialising[] = "SMP_LeaveInitialising";

  static constexpr EventId SMP_EnterStandbyId = 4;
  static constexpr Char8 SMP_EnterStandby[] = "SMP_EnterStandby";

  static constexpr EventId SMP_LeaveStandbyId = 5;
  static constexpr Char8 SMP_LeaveStandby[] = "SMP_LeaveStandby";

  static constexpr EventId SMP_EnterExecutingId = 6;
  static constexpr Char8 SMP_EnterExecuting[] = "SMP_EnterExecuting";

  static constexpr EventId SMP_LeaveExecutingId = 7;
  static constexpr Char8 SMP_LeaveExecuting[] = "SMP_LeaveExecuting";

  static constexpr EventId SMP_EnterStoringId = 8;
  static constexpr Char8 SMP_EnterStoring[] = "SMP_EnterStoring";

  static constexpr EventId SMP_LeaveStoringId = 9;
  static constexpr Char8 SMP_LeaveStoring[] = "SMP_LeaveStoring";

  static constexpr EventId SMP_EnterRestoringId = 10;
  static constexpr Char8 SMP_EnterRestoring[] = "SMP_EnterRestoring";

  static constexpr EventId SMP_LeaveRestoringId = 11;
  static constexpr Char8 SMP_LeaveRestoring[] = "SMP_LeaveRestoring";

  static constexpr EventId SMP_EnterExitingId = 12;
  static constexpr Char8 SMP_EnterExiting[] = "SMP_EnterExiting";

  static constexpr EventId SMP_EnterAbortingId = 13;
  static constexpr Char8 SMP_EnterAborting[] = "SMP_EnterAborting";

  static constexpr EventId SMP_EpochTimeChangedId = 14;
  static constexpr Char8 SMP_EpochTimeChanged[] = "SMP_EpochTimeChanged";

  static constexpr EventId SMP_MissionTimeChangedId = 15;
  static constexpr Char8 SMP_MissionTimeChanged[] = "SMP_MissionTimeChanged";

  static constexpr EventId SMP_EnterReconnectingId = 16;
  static constexpr Char8 SMP_EnterReconnecting[] = "SMP_EnterReconnecting";

  static constexpr EventId SMP_LeaveReconnectingId = 17;
  static constexpr Char8 SMP_LeaveReconnecting[] = "SMP_LeaveReconnecting";

  static constexpr EventId SMP_PreSimTimeChangeId = 18;
  static constexpr Char8 SMP_PreSimTimeChange[] = "SMP_PreSimTimeChange";

  static constexpr EventId SMP_PostSimTimeChangeId = 19;
  static constexpr Char8 SMP_PostSimTimeChange[] = "SMP_PostSimTimeChange";

  virtual ~IEventManager() noexcept = default;

  virtual EventId QueryEventId(String8 eventName) = 0;
  virtual void Subscribe(EventId event, const IEntryPoint *entryPoint) = 0;
  virtual void Unsubscribe(EventId event, const IEntryPoint *entryPoint) = 0;
  virtual void Emit(EventId event, Bool synchronous = true) = 0;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_IEVENTMANAGER_H_
