// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/IStringType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_ISTRINGTYPE_H_
#define SMP_PUBLICATION_ISTRINGTYPE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/IType.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for publication
    namespace Publication
    {
        /// This interface defines a user defined array type.
        class IStringType :
            public virtual Smp::Publication::IType
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~IStringType() noexcept = default;

            /// Get the maximum length of the string, excluding the last null
            /// character.
            /// @return  Size (number of array items) of the array type.
            virtual Smp::UInt64 GetMaxLength() const = 0;
        };
    }
}

#endif // SMP_PUBLICATION_ISTRINGTYPE_H_
