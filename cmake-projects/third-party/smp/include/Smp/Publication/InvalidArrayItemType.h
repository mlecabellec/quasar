// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/InvalidArrayItemType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_INVALIDARRAYITEMTYPE_H_
#define SMP_PUBLICATION_INVALIDARRAYITEMTYPE_H_

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
        /// This exception is raised when trying to register a simple array
        /// type, but the itemTypeUuid does not refer to a simple type.
        class InvalidArrayItemType : public virtual Smp::Exception
        {
        public:

            /// Virtual destructor to release memory.
            virtual ~InvalidArrayItemType() noexcept = default;

            /// Get the name of the invalid type that cannot be used.
            /// @return  Name of the new type that cannot be registered.
            virtual Smp::String8 GetTypeName() const noexcept = 0;

            /// Get the invalid type that cannot be used.
            /// @return  Type that uses the same Uuid.
            virtual Smp::PrimitiveTypeKind GetType() const noexcept = 0;
        };
    }
}

#endif // SMP_PUBLICATION_INVALIDARRAYITEMTYPE_H_
