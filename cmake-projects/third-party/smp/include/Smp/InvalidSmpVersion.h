// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidSmpVersion.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDSMPVERSION_H_
#define SMP_INVALIDSMPVERSION_H_

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
    /// This exception is raised when a Library exposes the wrong (or no) SMP
    /// Version.
    class InvalidSmpVersion : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidSmpVersion() noexcept = default;

        /// Get the version found in the Library, or 0 for none.
        /// @return  Version found in the Library, or 0 for none.
        virtual Smp::UInt64 GetLibrarySmpVersion() const noexcept = 0;
    };
}

#endif // SMP_INVALIDSMPVERSION_H_
