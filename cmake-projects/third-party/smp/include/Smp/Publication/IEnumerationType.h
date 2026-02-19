// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/IEnumerationType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_IENUMERATIONTYPE_H_
#define SMP_PUBLICATION_IENUMERATIONTYPE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/DuplicateName.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/DuplicateLiteral.h"
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
        /// This interface defines a user defined enumeration type.
        class IEnumerationType :
            public virtual Smp::Publication::IType
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~IEnumerationType() noexcept = default;

            /// Add a literal to the Enumeration.
            /// If the name is not a valid object name, an exception of type
            /// InvalidObjectName is thrown.
            /// If the name has already been used for another literal, an
            /// exception of type DuplicateName is thrown.
            /// If the value has already been used for another literal, a
            /// exception of type DuplicateLiteral is thrown.
            /// @param   name Name of the literal.
            /// @param   description Description of the literal.
            /// @param   value Value of the literal
            /// @throws  Smp::InvalidObjectName
            /// @throws  Smp::DuplicateName
            /// @throws  Smp::Publication::DuplicateLiteral
            virtual void AddLiteral(
                Smp::String8 name,
                Smp::String8 description,
                Smp::Int32 value) = 0;
        };
    }
}

#endif // SMP_PUBLICATION_IENUMERATIONTYPE_H_
