// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/NotReferenced.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_NOTREFERENCED_H_
#define SMP_NOTREFERENCED_H_

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
    class IComponent;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is thrown when trying to remove a component from a
    /// reference which was not referenced before.
    class NotReferenced : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~NotReferenced() noexcept = default;

        /// Get the name of the reference.
        /// @return  Name of the reference.
        virtual Smp::String8 GetReferenceName() const noexcept = 0;

        /// Get the Component that is not referenced.
        /// @return  Component that is not referenced.
        virtual const Smp::IComponent* GetComponent() const noexcept = 0;
    };
}

#endif // SMP_NOTREFERENCED_H_
