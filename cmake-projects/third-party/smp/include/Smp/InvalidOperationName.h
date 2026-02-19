// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidOperationName.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDOPERATIONNAME_H_
#define SMP_INVALIDOPERATIONNAME_H_

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
    /// This exception is raised by the Invoke() method when trying to invoke a
    /// method that does not exist, or that does not support dynamic invocation.
    class InvalidOperationName : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidOperationName() noexcept = default;

        /// Get the Operation name of request passed to the Invoke() method.
        /// @return  Operation name of request passed to the Invoke() method.
        virtual Smp::String8 GetOperationName() const noexcept = 0;
    };
}

#endif // SMP_INVALIDOPERATIONNAME_H_
