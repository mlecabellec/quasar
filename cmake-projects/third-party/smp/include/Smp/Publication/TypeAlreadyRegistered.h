// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Publication/TypeAlreadyRegistered.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_PUBLICATION_TYPEALREADYREGISTERED_H_
#define SMP_PUBLICATION_TYPEALREADYREGISTERED_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{

    namespace Publication
    {
        class IType;
    }
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Namespace for publication
    namespace Publication
    {
        /// This exception is raised when trying to register a type with a Uuid
        /// that has already been registered.
        class TypeAlreadyRegistered : public virtual Smp::Exception
        {
        public:

            /// Virtual destructor to release memory.
            virtual ~TypeAlreadyRegistered() noexcept = default;

            /// Get the name of the new type that cannot be registered.
            /// @return  Name of the new type that cannot be registered.
            virtual Smp::String8 GetTypeName() const noexcept = 0;

            /// Get the type that uses the same Uuid.
            /// @return  Type that uses the same Uuid.
            virtual const Smp::Publication::IType* GetType() const noexcept = 0;
        };
    }
}

#endif // SMP_PUBLICATION_TYPEALREADYREGISTERED_H_
