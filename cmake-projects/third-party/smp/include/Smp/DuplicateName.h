// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/DuplicateName.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_DUPLICATENAME_H_
#define SMP_DUPLICATENAME_H_

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
    /// This exception is raised when trying to add an object to a collection
    /// of objects, which have to have unique names, but another object with
    /// the same name does exist already in this collection. This would lead to
    /// duplicate names.
    class DuplicateName : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~DuplicateName() noexcept = default;

        /// Get the name that already exists in the collection.
        /// @return  Name that already exists in the collection.
        virtual Smp::String8 GetDuplicateName() const noexcept = 0;
    };
}

#endif // SMP_DUPLICATENAME_H_
