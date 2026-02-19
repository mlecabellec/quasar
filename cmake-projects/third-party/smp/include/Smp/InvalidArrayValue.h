// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidArrayValue.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDARRAYVALUE_H_
#define SMP_INVALIDARRAYVALUE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/InvalidFieldValue.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is raised when trying to assign an invalid value to an
    /// array field.
    class InvalidArrayValue : public virtual Smp::InvalidFieldValue
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidArrayValue() noexcept = default;

        /// Get the index in the array where the first invalid value was found.
        /// @return  Index in the array where the first invalid value was found.
        virtual Smp::Int64 GetInvalidValueIndex() const noexcept = 0;
    };
}

#endif // SMP_INVALIDARRAYVALUE_H_
