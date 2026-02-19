// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/RequestType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_REQUESTTYPE_H_
#define SMP_REQUESTTYPE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include <iosfwd>
#include "Smp/Int32.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// The Type of a Request defines if the component shall invoke an
    /// operation, retrieve a property, or set a property.
    enum class RequestType : Smp::Int32
    {
        /// Invoke and Operation.
        RT_Invoke,

        /// Get a Property.
        RT_Get,

        /// Set a Property.
        RT_Set
    };

    /// Stream operator for RequestType to be able to print an enum value.
    /// @param os Output stream to output to.
    /// @param obj Object to output to stream.
    std::ostream& operator << (std::ostream& os, const RequestType& obj);
}

#endif // SMP_REQUESTTYPE_H_
