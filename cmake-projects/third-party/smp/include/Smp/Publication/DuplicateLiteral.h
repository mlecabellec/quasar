// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/DuplicateLiteral.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_DUPLICATELITERAL_H_
#define SMP_PUBLICATION_DUPLICATELITERAL_H_

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
    /// Namespace for publication
    namespace Publication
    {
        /// This exception is raised when trying to add a literal to an
        /// enumeration using a value that has been used for another literal
        /// before.
        class DuplicateLiteral : public virtual Smp::Exception
        {
        public:

            /// Virtual destructor to release memory.
            virtual ~DuplicateLiteral() noexcept = default;

            /// Get the name of the literal that has been added before using
            /// the same value.
            /// @return  Name of the literal.
            virtual Smp::String8 GetLiteralName() const noexcept = 0;

            /// Get the value of the literal that has been used before.
            /// @return  Value of the literal.
            virtual Smp::Int32 GetLiteralValue() const noexcept = 0;
        };
    }
}

#endif // SMP_PUBLICATION_DUPLICATELITERAL_H_
