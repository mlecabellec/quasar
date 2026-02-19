// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/NoDynamicInvocation.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_NODYNAMICINVOCATION_H_
#define SMP_NODYNAMICINVOCATION_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IComponent;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is thrown when trying to publish an operation or
    /// property for a component that is not implementing IDynamicInvocation.
    class NoDynamicInvocation : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~NoDynamicInvocation() noexcept = default;

        /// Get the Component that does not implement IDynamicInvocation.
        /// @return  Component that does not implement IDynamicInvocation.
        virtual const Smp::IComponent* GetComponent() const noexcept = 0;
    };
}

#endif // SMP_NODYNAMICINVOCATION_H_
