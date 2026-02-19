// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidFieldValue.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDFIELDVALUE_H_
#define SMP_INVALIDFIELDVALUE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/InvalidAnyType.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IField;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is raised when trying to assign an invalid value to a
    /// field.
    class InvalidFieldValue : public virtual Smp::InvalidAnyType
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidFieldValue() noexcept = default;

        /// Get the field that rejected the value.
        /// @return  Field that rejected the value.
        virtual const Smp::IField* GetField() const noexcept = 0;
    };
}

#endif // SMP_INVALIDFIELDVALUE_H_
