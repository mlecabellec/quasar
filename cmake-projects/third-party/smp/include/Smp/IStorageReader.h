// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IStorageReader.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ISTORAGEREADER_H_
#define SMP_ISTORAGEREADER_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/PrimitiveTypes.h"
#include "Smp/Void.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This interface provides functionality to read data from storage.
    /// @remarks A client (typically the simulation environment) provides this
    /// interface to allow objects implementing the IPersist interface to
    /// restore their state. It is passed to the Restore() method of every
    /// object implementing IPersist.
    class IStorageReader
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IStorageReader() noexcept = default;

        /// Restore data from storage.
        /// This method reads a memory block of data from the state vector. It
        /// is the object's responsibility to Store a block of the same size to
        /// IStorageWriter on Store.
        /// @param   address Memory address of memory block.
        /// @param   size Size of memory block.
        virtual void Restore(
            Smp::Void* address,
            Smp::UInt64 size) = 0;

        /// Get the state vector file name.
        /// This method returns a full file name, including the path to the
        /// file.
        /// @return  This method returns the full path of the state vector file
        /// currently loaded by the Storage Reader.
        virtual Smp::String8 GetStateVectorFileName() const = 0;

        /// Get the state vector file path.
        /// This method returns a full file path that shall be used when
        /// reading additional files that correspond to the state vector file
        /// read by the Storage Reader.
        /// @return  This method returns the full path for auxiliary files
        /// corresponding to the state vector file currently loaded by the
        /// Storage Reader.
        virtual Smp::String8 GetStateVectorFilePath() const = 0;
    };
}

#endif // SMP_ISTORAGEREADER_H_
