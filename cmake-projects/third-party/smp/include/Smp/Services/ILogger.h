// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Services/ILogger.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_SERVICES_ILOGGER_H_
#define SMP_SERVICES_ILOGGER_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/IService.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Services/LogMessageKind.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IObject;
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
        /// This interface gives access to the Logger Service.
        /// All objects in a simulation can log messages using this service.
        /// Objects can either use pre-defined log message kinds, or define
        /// their own message kinds.
        class ILogger :
            public virtual Smp::IService
        {
        public:
            /// The message contains general information.
            static constexpr Smp::Services::LogMessageKind LMK_Information = 0;

            /// The message has been sent from an event, typically from a state
            /// transition.
            static constexpr Smp::Services::LogMessageKind LMK_Event = 1;

            /// The message contains a warning.
            static constexpr Smp::Services::LogMessageKind LMK_Warning = 2;

            /// The message has been raised because of an error.
            static constexpr Smp::Services::LogMessageKind LMK_Error = 3;

            /// The message contains debug information.
            static constexpr Smp::Services::LogMessageKind LMK_Debug = 4;

            /// The message contains general information.
            static constexpr Smp::Char8 LMK_InformationName[] = "Information";

            /// The message contains debug information.
            static constexpr Smp::Char8 LMK_DebugName[] = "Debug";

            /// The message has been raised because of an error.
            static constexpr Smp::Char8 LMK_ErrorName[] = "Error";

            /// The message contains a warning.
            static constexpr Smp::Char8 LMK_WarningName[] = "Warning";

            /// The message has been sent from an event, typically from a state
            /// transition.
            static constexpr Smp::Char8 LMK_EventName[] = "Event";

            /// Virtual destructor to release memory.
            virtual ~ILogger() noexcept = default;

            /// Return identifier of log message kind by name.
            /// It is guaranteed that this method always returns the same id
            /// for the same messageKindName string.
            /// @remarks This method can be used for predefined log message
            /// kinds, but is especially useful for user-defined log message
            /// kinds.
            /// @param   messageKindName Name of log message kind.
            /// @return  Identifier of log message kind.
            virtual Smp::Services::LogMessageKind QueryLogMessageKind(
                Smp::String8 messageKindName) = 0;

            /// This function logs a message to the simulation log.
            /// @param   sender Object that sends the message.
            /// @param   message The message to log.
            /// @param   kind Kind of message.
            virtual void Log(
                const Smp::IObject* sender,
                Smp::String8 message,
                Smp::Services::LogMessageKind kind = 0) = 0;
        };
    }
}

#endif // SMP_SERVICES_ILOGGER_H_
