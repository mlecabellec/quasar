// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidFile.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDFILE_H_
#define SMP_INVALIDFILE_H_

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
    /// This exception is raised when trying to load a file that does not
    /// conform to its specification.
    class InvalidFile : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidFile() noexcept = default;

        /// Get the file name of the file that is invalid.
        /// @return  File name of the file that is invalid.
        virtual Smp::String8 GetFileName() const noexcept = 0;

        /// Get a textual description of the error encountered.
        /// @return  Textual description of the error encountered.
        virtual Smp::String8 GetErrorMessage() const noexcept = 0;
    };
}

#endif // SMP_INVALIDFILE_H_
