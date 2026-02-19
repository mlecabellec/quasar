// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/IArrayType.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_IARRAYTYPE_H_
#define SMP_PUBLICATION_IARRAYTYPE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/IType.h"
#include "Smp/Publication/TypeNotRegistered.h"

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
        class IArrayType :
            public virtual Smp::Publication::IType
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~IArrayType() noexcept = default;

            /// Get the size (number of array items) of the array type.
            /// @return  Size (number of array items) of the array type.
            virtual Smp::UInt64 GetSize() const = 0;

            /// Get the type of each array item.
            /// Within one array, each array item needs to be of the same type.
            /// If the item type is not found in the Type Registry, the
            /// exception TypeNotRegistered is raised.
            /// @return  Type of each array item.
            /// @throws  Smp::Publication::TypeNotRegistered
            virtual const Smp::Publication::IType* GetItemType() const = 0;
        };
    }
}

#endif // SMP_PUBLICATION_IARRAYTYPE_H_
