// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/InvalidPrimitiveType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_INVALIDPRIMITIVETYPE_H_
#define SMP_PUBLICATION_INVALIDPRIMITIVETYPE_H_

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
        /// This exception is raised when trying to use an invalid primitive
        /// type kind as parameter for a user-defined float or integer type.
        class InvalidPrimitiveType : public virtual Smp::Exception
        {
        public:

            /// Virtual destructor to release memory.
            virtual ~InvalidPrimitiveType() noexcept = default;

            /// Get the name of the invalid type that cannot be used.
            /// @return  Name of the invalid type that cannot be used.
            virtual Smp::String8 GetTypeName() const noexcept = 0;

            /// Get the invalid type that cannot be used.
            /// @return  Invalid type that cannot be used.
            virtual Smp::PrimitiveTypeKind GetType() const noexcept = 0;
        };
    }
}

#endif // SMP_PUBLICATION_INVALIDPRIMITIVETYPE_H_
