// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidSimulatorState.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDSIMULATORSTATE_H_
#define SMP_INVALIDSIMULATORSTATE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"
#include "Smp/SimulatorStateKind.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is raised by the simulator when one of the operations is
    /// called in an invalid state.
    class InvalidSimulatorState : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidSimulatorState() noexcept = default;

        /// Get the invalid state in which an operation call was made.
        /// @return  Invalid state in which an operation call was made.
        virtual Smp::SimulatorStateKind GetInvalidState() const noexcept = 0;
    };
}

#endif // SMP_INVALIDSIMULATORSTATE_H_
