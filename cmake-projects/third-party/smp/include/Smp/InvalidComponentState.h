// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidComponentState.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDCOMPONENTSTATE_H_
#define SMP_INVALIDCOMPONENTSTATE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/ComponentStateKind.h"
#include "Smp/Exception.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is raised by a component when one of the state
    /// transition commands is called in an invalid state.
    class InvalidComponentState : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidComponentState() noexcept = default;

        /// Get the invalid state in which a state transition was attempted.
        /// @return  Invalid state in which a state transition was attempted.
        virtual Smp::ComponentStateKind GetInvalidState() const noexcept = 0;

        /// Get the expected state of the component when calling this state
        /// transition.
        /// @return  Expected state of the component when calling this state
        /// transition.
        virtual Smp::ComponentStateKind GetExpectedState() const noexcept = 0;
    };
}

#endif // SMP_INVALIDCOMPONENTSTATE_H_
