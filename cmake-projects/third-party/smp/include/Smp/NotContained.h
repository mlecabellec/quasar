// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/NotContained.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_NOTCONTAINED_H_
#define SMP_NOTCONTAINED_H_

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
    /// This exception is thrown when trying to delete a component from a
    /// container which is not contained.
    class NotContained : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~NotContained() noexcept = default;

        /// Get the name of the container.
        /// @return  Name of the container.
        virtual Smp::String8 GetContainerName() const noexcept = 0;

        /// Get the Component that is not contained.
        /// @return  Component that is not contained.
        virtual const Smp::IComponent* GetComponent() const noexcept = 0;
    };
}

#endif // SMP_NOTCONTAINED_H_
