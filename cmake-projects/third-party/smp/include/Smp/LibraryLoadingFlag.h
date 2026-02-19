// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/LibraryLoadingFlag.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_LIBRARYLOADINGFLAG_H_
#define SMP_LIBRARYLOADINGFLAG_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include <iosfwd>
#include "Smp/Int32.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This flag defines whether the symbols in a library shall be loaded as
    /// local symbols, global symbolds, or using the default setting of the
    /// simulation infrastructure.
    enum class LibraryLoadingFlag : Smp::Int32
    {
        /// Load the library using the default setting of the simulation
        /// environment.
        LLF_Auto,

        /// Load the library with global flag, making the symbols available
        /// globally.
        LLF_Global,

        /// Load the library with local flag, making the symbols only available
        /// locally within the library.
        LLF_Local
    };

    /// Stream operator for LibraryLoadingFlag to be able to print an enum value.
    /// @param os Output stream to output to.
    /// @param obj Object to output to stream.
    std::ostream& operator << (std::ostream& os, const LibraryLoadingFlag& obj);
}

#endif // SMP_LIBRARYLOADINGFLAG_H_
