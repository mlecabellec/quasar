// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidAnyType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDANYTYPE_H_
#define SMP_INVALIDANYTYPE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/AnySimple.h"
#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is raised when trying to use an AnySimple argument of
    /// wrong type.
    /// @remarks This can happen when assigning an AnySimple value to a Field,
    /// Property, or Parameter.
    class InvalidAnyType : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidAnyType() noexcept = default;

        /// Get the value that is not valid.
        /// @return  Value that is not valid.
        virtual Smp::AnySimple GetInvalidValue() const noexcept = 0;

        /// Get the type that was expected.
        /// @return  Type that was expected.
        virtual Smp::PrimitiveTypeKind GetExpectedType() const noexcept = 0;
    };
}

#endif // SMP_INVALIDANYTYPE_H_
