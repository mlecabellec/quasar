// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Services/InvalidSimulationTime.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_SERVICES_INVALIDSIMULATIONTIME_H_
#define SMP_SERVICES_INVALIDSIMULATIONTIME_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for simulation services
    namespace Services
    {
        /// This exception is thrown by SetSimulationTime if the new simulation
        /// time is not between the current simulation time and the simulation
        /// time of the next event on the scheduler.
        class InvalidSimulationTime : public virtual Smp::Exception
        {
        public:

            /// Virtual destructor to release memory.
            virtual ~InvalidSimulationTime() noexcept = default;

            /// Get the current simulation time maintained by the time keeper.
            /// @return  Current simulation time maintained by the time keeper.
            virtual Smp::Duration GetCurrentTime() const noexcept = 0;

            /// Get the simulation time provided to SetSimulationTime.
            /// @return  Simulation time provided to SetSimulationTime.
            virtual Smp::Duration GetProvidedTime() const noexcept = 0;

            /// Get the maximum simulation time that can be set using
            /// SetSimulationTime.
            /// @return  Maximum simulation time that can be set using
            /// SetSimulationTime.
            virtual Smp::Duration GetMaximumTime() const noexcept = 0;
        };
    }
}

#endif // SMP_SERVICES_INVALIDSIMULATIONTIME_H_
