// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Services/IEventManager.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_SERVICES_IEVENTMANAGER_H_
#define SMP_SERVICES_IEVENTMANAGER_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/IService.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Services/EntryPointAlreadySubscribed.h"
#include "Smp/Services/EntryPointNotSubscribed.h"
#include "Smp/Services/EventId.h"
#include "Smp/Services/InvalidEventId.h"
#include "Smp/Services/InvalidEventName.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IEntryPoint;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for simulation services
    namespace Services
    {
        /// This interface is implemented by the Event Manager Service.
        /// Components can register entry points with events, and they can
        /// define and emit events.
        class IEventManager :
            public virtual Smp::IService
        {
        public:
            /// This event is raised when leaving the Connecting state with an
            /// automatic state transition to Initializing state.
            static constexpr Smp::Services::EventId SMP_LeaveConnectingId = 1;

            /// Leave Connecting state.
            /// When leaving the Connecting state with an automatic state
            /// transition to Initializing state
            static constexpr Smp::Char8 SMP_LeaveConnecting[] = "SMP_LeaveConnecting";

            /// This event is raised when entering the Initialising state with
            /// an automatic state transition from Connecting state, or with
            /// the Initialise() state transition. 
            static constexpr Smp::Services::EventId SMP_EnterInitialisingId = 2;

            /// Enter Initialising state.
            /// When entering the Initialising state with an automatic state
            /// transition from Connecting state, or with the Initialise()
            /// state transition.
            static constexpr Smp::Char8 SMP_EnterInitialising[] = "SMP_EnterInitialising";

            /// This event is raised when leaving the Initialising state with
            /// an automatic state transition to Standby state.
            static constexpr Smp::Services::EventId SMP_LeaveInitialisingId = 3;

            /// Leave Initialising state.
            /// When leaving the Initialising state with an automatic state
            /// transition to Standby state.
            static constexpr Smp::Char8 SMP_LeaveInitialising[] = "SMP_LeaveInitialising";

            /// This event is raised when entering the Standby state with an
            /// automatic state transition from Initialising, Storing or
            /// Restoring state, or with the Hold() state transition command
            /// from Executing state. 
            static constexpr Smp::Services::EventId SMP_EnterStandbyId = 4;

            /// Enter Standby state.
            /// When entering the Standby state with an automatic state
            /// transition from Initialising, Storing or Restoring state, or
            /// the Hold() state transition command from Executing state.
            static constexpr Smp::Char8 SMP_EnterStandby[] = "SMP_EnterStandby";

            /// This event is raised when leaving the Standby state with the
            /// Run() state transition command to Executing state, with the
            /// Store() state transition command to Storing state, with the
            /// Restore() state transition command to Restoring state, or with
            /// the Initialise() state transition command to Initialising
            /// state. 
            static constexpr Smp::Services::EventId SMP_LeaveStandbyId = 5;

            /// Leave Standby state.
            /// When leaving the Standby state with
            /// <ul>
            /// <li>the Run() state transition command to Executing state,
            /// <li>the Store() state transition command to Storing state, 
            /// <li>the Restore() state transition command to Restoring state,
            /// <li>the Initialise() state transition command to Initialising
            /// state.
            /// </ul>
            static constexpr Smp::Char8 SMP_LeaveStandby[] = "SMP_LeaveStandby";

            /// This event is raised when entering the Executing state with the
            /// Run() state transition command from Standby state.
            static constexpr Smp::Services::EventId SMP_EnterExecutingId = 6;

            /// Enter Executing state.
            /// When entering the Executing state with the Run() state
            /// transition command from Standby state.
            static constexpr Smp::Char8 SMP_EnterExecuting[] = "SMP_EnterExecuting";

            /// This event is raised when leaving the Executing state with the
            /// Hold() state transition command to Standby state.
            static constexpr Smp::Services::EventId SMP_LeaveExecutingId = 7;

            /// Leave Executing state.
            /// When leaving the Executing state with the Hold() state
            /// transition command to Standby state.
            static constexpr Smp::Char8 SMP_LeaveExecuting[] = "SMP_LeaveExecuting";

            /// This event is raised when entering the Storing state with the
            /// Store() state transition command from Standby state.
            static constexpr Smp::Services::EventId SMP_EnterStoringId = 8;

            /// Enter Storing state.
            /// When entering the Storing state with the Store() state
            /// transition command from Standby state.
            static constexpr Smp::Char8 SMP_EnterStoring[] = "SMP_EnterStoring";

            /// This event is raised when leaving the Storing state with an
            /// automatic state transition to Standby state.
            static constexpr Smp::Services::EventId SMP_LeaveStoringId = 9;

            /// Leave Storing state.
            /// When leaving the Storing state with an automatic state
            /// transition to Standby state.
            static constexpr Smp::Char8 SMP_LeaveStoring[] = "SMP_LeaveStoring";

            /// This event is raised when entering the Restoring state with the
            /// Restore() state transition command from Standby state.
            static constexpr Smp::Services::EventId SMP_EnterRestoringId = 10;

            /// Enter Restoring state.
            /// When entering the Restoring state with the Restore() state
            /// transition command from Standby state.
            static constexpr Smp::Char8 SMP_EnterRestoring[] = "SMP_EnterRestoring";

            /// This event is raised when leaving the Restoring state with an
            /// automatic state transition to Standby state.
            static constexpr Smp::Services::EventId SMP_LeaveRestoringId = 11;

            /// Leave Restoring state.
            /// When leaving the Restoring state with an automatic state
            /// transition to Standby state.
            static constexpr Smp::Char8 SMP_LeaveRestoring[] = "SMP_LeaveRestoring";

            /// This event is raised when entering the Exiting state with the
            /// Exit() state transition command from Standby state.
            static constexpr Smp::Services::EventId SMP_EnterExitingId = 12;

            /// Enter Exiting state.
            /// When entering the Exiting state with the Exit() state
            /// transition command from Standby state.
            static constexpr Smp::Char8 SMP_EnterExiting[] = "SMP_EnterExiting";

            /// This event is raised when entering the Aborting state with the
            /// Abort() state transition command from any other state.
            static constexpr Smp::Services::EventId SMP_EnterAbortingId = 13;

            /// Enter Aborting state.
            /// When entering the Aborting state with the Abort() state
            /// transition command from any other state.
            static constexpr Smp::Char8 SMP_EnterAborting[] = "SMP_EnterAborting";

            /// This event is raised when changing the epoch time with the
            /// SetEpochTime() method of the time keeper service.
            static constexpr Smp::Services::EventId SMP_EpochTimeChangedId = 14;

            /// Epoch Time has changed.
            /// When changing the epoch time with the SetEpochTime() method of
            /// the time keeper service.
            static constexpr Smp::Char8 SMP_EpochTimeChanged[] = "SMP_EpochTimeChanged";

            /// This event is raised when changing the mission time with one of
            /// the SetMissionTime() and SetMissionStart() methods of the time
            /// keeper service. 
            static constexpr Smp::Services::EventId SMP_MissionTimeChangedId = 15;

            /// Mission time has changed.
            /// When changing the mission time with one of the SetMissionTime()
            /// and SetMissionStartTime() methods of the time keeper service.
            static constexpr Smp::Char8 SMP_MissionTimeChanged[] = "SMP_MissionTimeChanged";

            /// This event is raised when entering the Reconnecting state with
            /// the Reconnect() state transition from Standby state.
            static constexpr Smp::Services::EventId SMP_EnterReconnectingId = 16;

            /// Enter Reconnecting state.
            /// When entering the Reconnecting state with the Reconnect() state
            /// transition from Standby state.
            static constexpr Smp::Char8 SMP_EnterReconnecting[] = "SMP_EnterReconnecting";

            /// This event is raised when leaving the Reconnecting state with
            /// an automatic state transition to Standby state.
            static constexpr Smp::Services::EventId SMP_LeaveReconnectingId = 17;

            /// Leave Reconnecting state.
            /// When leaving the Reconnecting state with an automatic state
            /// transition to Standby state.
            static constexpr Smp::Char8 SMP_LeaveReconnecting[] = "SMP_LeaveReconnecting";

            /// This event is raised before the Time Keeper updates the
            /// Simulation Time.
            static constexpr Smp::Services::EventId SMP_PreSimTimeChangeId = 18;

            /// Before changing the Simulation Time.
            /// When all events have been executed by the Scheduler for a
            /// specific Simulation Time, but before the TimeKeeper changes the
            /// Simulation time to the time of next event.
            static constexpr Smp::Char8 SMP_PreSimTimeChange[] = "SMP_PreSimTimeChange";

            /// This event is raised after the simulation time has been changed
            /// by the Time Keeper, but before an event has been executed by
            /// the Scheduler.
            static constexpr Smp::Services::EventId SMP_PostSimTimeChangeId = 19;

            /// After changing the Simulation Time.
            /// When the simulation time has been changed by the Time Keeper,
            /// but before any events have been executed by the Scheduler.
            static constexpr Smp::Char8 SMP_PostSimTimeChange[] = "SMP_PostSimTimeChange";

            /// Virtual destructor to release memory.
            virtual ~IEventManager() noexcept = default;

            /// Get unique event identifier for an event name.
            /// It is guaranteed that this method will always return the same
            /// value when called with the same event name. This holds for
            /// pre-defined event names as well as for user-defined events.
            /// @param   eventName Name of the global event.
            /// @return  Event identifier for global event with given name.
            /// @throws  Smp::Services::InvalidEventName
            virtual Smp::Services::EventId QueryEventId(
                Smp::String8 eventName) = 0;

            /// Subscribe entry point to a global event.
            /// An entry point can only be subscribed once to an event.
            /// This method raises an exception of type InvalidEventId when
            /// called with an invalid event identifier. When the entry point
            /// is already subscribed to the same event, an exception of type
            /// EntryPointAlreadySubscribed is raised.
            /// @param   event Event identifier of global event to subscribe to.
            /// @param   entryPoint Entry point to subscribe to global event.
            /// @throws  Smp::Services::InvalidEventId
            /// @throws  Smp::Services::EntryPointAlreadySubscribed
            virtual void Subscribe(
                Smp::Services::EventId event,
                const Smp::IEntryPoint* entryPoint) = 0;

            /// Unsubscribe entry point from a global event.
            /// An entry point can only be unsubscribed from an event when it
            /// has been subscribed earlier using Subscribe().
            /// This method raises an exception of type InvalidEventId when
            /// called with an invalid event identifier. When the entry point
            /// is not subscribed to the event, an exception of type
            /// EntryPointNotSubscribed is raised.
            /// @param   event Event identifier of global event to unsubscribe
            /// from.
            /// @param   entryPoint Entry point to unsubscribe from global
            /// event.
            /// @throws  Smp::Services::InvalidEventId
            /// @throws  Smp::Services::EntryPointNotSubscribed
            virtual void Unsubscribe(
                Smp::Services::EventId event,
                const Smp::IEntryPoint* entryPoint) = 0;

            /// Emit a global event.
            /// Only the simulation environment itself is allowed to emit
            /// events for changes of the simulator state. While an event for
            /// such a state transition is emitted, subscribed event handlers
            /// are not allowed to call another state transition of the
            /// simulator.
            /// This will call all entry points that are subscribed to the
            /// global event with the given identifier at the time Emit() is
            /// called. Entry point subscription/unsubscription during the
            /// execution of Emit() is taken into account the next time Emit()
            /// is called. Entry points will be called in the order they have
            /// been subscribed to the global event.
            /// @param   event Event identifier of global event to emit.
            /// @param   synchronous Flag whether to emit the given event
            /// synchronously (the default) or asynchronously.
            /// @throws  Smp::Services::InvalidEventId
            virtual void Emit(
                Smp::Services::EventId event,
                Smp::Bool synchronous = true) = 0;
        };
    }
}

#endif // SMP_SERVICES_IEVENTMANAGER_H_
