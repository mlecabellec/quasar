// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidParent.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDPARENT_H_
#define SMP_INVALIDPARENT_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IObject;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is raised when trying to add a child component to a
    /// parent component (via AddComponent of IContainer), but the child's
    /// parent does not match parent component.
    class InvalidParent : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidParent() noexcept = default;

        /// Get the parent that the component exposes via GetParent().
        /// @return  Field for which the Connect operation was called
        virtual const Smp::IObject* GetParentFound() const noexcept = 0;

        /// Get the parent that the component was added to (i.e. the expected
        /// parent).
        /// @return  Parent that the component was added to.
        virtual const Smp::IObject* GetParentExpected() const noexcept = 0;
    };
}

#endif // SMP_INVALIDPARENT_H_
