// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IEntryPoint.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IENTRYPOINT_H_
#define SMP_IENTRYPOINT_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/IObject.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface of an entry point.
    /// This interface provides a notification method (event handler) that can
    /// be called e.g. by the Scheduler or Event Manager when an event is
    /// emitted.
    class IEntryPoint :
        public virtual Smp::IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IEntryPoint() noexcept = default;

        /// This method shall be called when an associated event is emitted.
        virtual void Execute() const = 0;
    };
}

#endif // SMP_IENTRYPOINT_H_
