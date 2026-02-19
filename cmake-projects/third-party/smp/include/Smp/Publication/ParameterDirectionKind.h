// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/ParameterDirectionKind.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_PARAMETERDIRECTIONKIND_H_
#define SMP_PUBLICATION_PARAMETERDIRECTIONKIND_H_

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
    /// Namespace for publication
    namespace Publication
    {
        /// The Parameter Direction Kind enumeration defines the possible
        /// parameter directions.
        enum class ParameterDirectionKind : Smp::Int32
        {
            /// The parameter is read-only to the operation, i.e. its value
            /// must be specified on call, and cannot be changed inside the
            /// operation.
            PDK_In,

            /// The parameter is write-only to the operation, i.e. its value is
            /// unspecified on call, and must be set by the operation.
            PDK_Out,

            /// The parameter must be specified on call, and may be changed by
            /// the operation.
            PDK_InOut,

            /// The parameter represents the operation's return value.
            PDK_Return
        };

        /// Stream operator for ParameterDirectionKind to be able to print an enum value.
        /// @param os Output stream to output to.
        /// @param obj Object to output to stream.
        std::ostream& operator << (std::ostream& os, const ParameterDirectionKind& obj);
    }
}

#endif // SMP_PUBLICATION_PARAMETERDIRECTIONKIND_H_
