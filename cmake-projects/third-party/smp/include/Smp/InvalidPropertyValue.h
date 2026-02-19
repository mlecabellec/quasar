// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidPropertyValue.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDPROPERTYVALUE_H_
#define SMP_INVALIDPROPERTYVALUE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/InvalidAnyType.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IProperty;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is raised when trying to assign an invalid value to a
    /// property.
    class InvalidPropertyValue : public virtual Smp::InvalidAnyType
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidPropertyValue() noexcept = default;

        /// Get the property that rejected the value.
        /// @return  The property that rejected the value.
        virtual const Smp::IProperty* GetProperty() const noexcept = 0;
    };
}

#endif // SMP_INVALIDPROPERTYVALUE_H_
