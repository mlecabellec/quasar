// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/FileNotFound.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_FILENOTFOUND_H_
#define SMP_FILENOTFOUND_H_

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
    /// This exception is raised when trying to load a file that does not exist.
    class FileNotFound : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~FileNotFound() noexcept = default;

        /// Get the file name of the file that is not found
        /// @return  File name that was not valid.
        virtual Smp::String8 GetFileName() const noexcept = 0;
    };
}

#endif // SMP_FILENOTFOUND_H_
