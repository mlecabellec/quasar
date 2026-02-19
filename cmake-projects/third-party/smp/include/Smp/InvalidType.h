// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDTYPE_H_
#define SMP_INVALIDTYPE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Invalid field type.
    /// This exception is raised when trying to publish an object with invalid
    /// type.
    /// @remarks This can happen, for example: - when trying to publish a field
    /// of the variable-length simple type String8. - when trying to publish a
    /// property with a type with primitive type kind None. - when trying to
    /// publish an operation parameter with a type with primitive type kind
    /// None.
    class InvalidType : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidType() noexcept = default;
    };
}

#endif // SMP_INVALIDTYPE_H_
