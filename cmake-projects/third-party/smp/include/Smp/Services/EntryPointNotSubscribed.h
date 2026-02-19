// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Services/EntryPointNotSubscribed.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_SERVICES_ENTRYPOINTNOTSUBSCRIBED_H_
#define SMP_SERVICES_ENTRYPOINTNOTSUBSCRIBED_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

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
        /// This exception is raised when trying to unsubscribe an entry point
        /// from an event that is not subscribed to it.
        class EntryPointNotSubscribed : public virtual Smp::Exception
        {
        public:

            /// Virtual destructor to release memory.
            virtual ~EntryPointNotSubscribed() noexcept = default;

            /// Get the entry point that is not subscribed to the event.
            /// @return  Entry point that is not subscribed to the event.
            virtual const Smp::IEntryPoint* GetEntryPoint() const noexcept = 0;

            /// Get the name of the event that the entry point is not
            /// subscribed to.
            /// @return  Name of the event that the entry point is not
            /// subscribed to.
            virtual Smp::String8 GetEventName() const noexcept = 0;
        };
    }
}

#endif // SMP_SERVICES_ENTRYPOINTNOTSUBSCRIBED_H_
