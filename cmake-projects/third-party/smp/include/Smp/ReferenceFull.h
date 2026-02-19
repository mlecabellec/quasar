// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/ReferenceFull.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_REFERENCEFULL_H_
#define SMP_REFERENCEFULL_H_

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
    /// This exception is raised when trying to add a component to a reference
    /// that is full, i.e. where the Count has reached the Upper limit.
    class ReferenceFull : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~ReferenceFull() noexcept = default;

        /// Get the name of the reference.
        /// @return  Name of the reference.
        virtual Smp::String8 GetReferenceName() const noexcept = 0;

        /// Get the number of components in the reference, which is its Upper
        /// limit when the reference is full.
        /// @return  Number of components in the reference, which is its Upper
        /// limit when the reference is full.
        virtual Smp::Int64 GetReferenceSize() const noexcept = 0;
    };
}

#endif // SMP_REFERENCEFULL_H_
