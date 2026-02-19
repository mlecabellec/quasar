// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidObjectName.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDOBJECTNAME_H_
#define SMP_INVALIDOBJECTNAME_H_

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
    /// This exception is raised when trying to set an object's name to an
    /// invalid name. Names
    /// <ul>
    /// <li>must not be empty,</li>
    /// <li>must start with a letter, and</li>
    /// <li>must only contain letters, digits, the underscore ("_") and
    /// brackets ("[" and "]").</li>
    /// </ul>
    class InvalidObjectName : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidObjectName() noexcept = default;

        /// Get the invalid object name passed to the constructor of the object.
        /// @return  Invalid object name passed to the constructor of the
        /// object.
        virtual Smp::String8 GetInvalidName() const noexcept = 0;
    };
}

#endif // SMP_INVALIDOBJECTNAME_H_
