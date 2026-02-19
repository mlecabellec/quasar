// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Services/InvalidEventId.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_SERVICES_INVALIDEVENTID_H_
#define SMP_SERVICES_INVALIDEVENTID_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"
#include "Smp/Services/EventId.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for simulation services
    namespace Services
    {
        /// This exception is raised when an invalid event id is provided, e.g.
        /// when calling Subscribe(), Unsubscribe() or Emit() of the Event
        /// Manager (using an invalid global event id), or when calling
        /// SetEventSimulationTime(), SetEventMissionTime(),
        /// SetEventEpochTime(), SetEventZuluTime(), SetEventCycleTime(),
        /// SetEventRepeat() or RemoveEvent() of the Scheduler (using an
        /// invalid scheduler event id).
        class InvalidEventId : public virtual Smp::Exception
        {
        public:

            /// Virtual destructor to release memory.
            virtual ~InvalidEventId() noexcept = default;

            /// Get the invalid event identifier.
            /// @return  Invalid event identifier.
            virtual Smp::Services::EventId GetInvalidEventId() const noexcept = 0;
        };
    }
}

#endif // SMP_SERVICES_INVALIDEVENTID_H_
