// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/ViewKind.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_VIEWKIND_H_
#define SMP_VIEWKIND_H_

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
    /// This enumeration defines possible options for the View attribute, which
    /// can be used to control if and how an element is made visible when
    /// published to the simulation infrastructure.
    /// The simulation infrastructure must at least support the "None" and the
    /// "All" roles (i.e. hidden or always visible).
    /// The simulation infrastructure may support the selection of different
    /// user roles, in which case the "Debug" and the "Expert" role must also
    /// be supported as described.
    enum class ViewKind : Smp::Int32
    {
        /// The element is not made visible to the user (this is the default).
        VK_None,

        /// The element is made visible for debugging purposes.
        /// The element is not visible to end users. If the simulation
        /// infrastructure supports the selection of different user roles, then
        /// the element shall be visible to "Debug" users only.
        VK_Debug,

        /// The element is made visible for expert users.
        /// The element is not visible to end users. If the simulation
        /// infrastructure supports the selection of different user roles, then
        /// the element shall be visible to "Debug" and "Expert" users.
        VK_Expert,

        /// The element is made visible to all users.
        VK_All
    };

    /// Stream operator for ViewKind to be able to print an enum value.
    /// @param os Output stream to output to.
    /// @param obj Object to output to stream.
    std::ostream& operator << (std::ostream& os, const ViewKind& obj);
}

#endif // SMP_VIEWKIND_H_
