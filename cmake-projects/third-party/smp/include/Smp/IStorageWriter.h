// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IStorageWriter.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ISTORAGEWRITER_H_
#define SMP_ISTORAGEWRITER_H_

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
    /// This interface provides functionality to write data to storage.
    /// @remarks A client (typically the simulation environment) provides this
    /// interface to allow objects implementing the IPersist interface to store
    /// their state. It is passed to the Store() method of every object
    /// implementing IPersist.
    class IStorageWriter
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IStorageWriter() noexcept = default;

        /// Store data to storage.
        /// This method writes a memory block of data to the state vector. It
        /// is the object's responsibility to Restore a block of the same size
        /// from IStorageReader on Restore.
        /// @param   address Memory address of memory block.
        /// @param   size Size of memory block.
        virtual void Store(
            const Smp::Void* address,
            Smp::UInt64 size) = 0;

        /// Get the state vector file name.
        /// This method returns a full file name, including the path to the
        /// file.
        /// @return  This method returns the full path of the state vector file
        /// currently stored by the Storage Writer.
        virtual Smp::String8 GetStateVectorFileName() const = 0;

        /// Get the state vector file path.
        /// This method returns a full file path that shall be used when
        /// writing additional files that correspond to the state vector file
        /// written by the Storage Writer.
        /// @return  This method returns the full path for auxiliary files
        /// corresponding to the state vector file currently stored by the
        /// Storage Writer.
        virtual Smp::String8 GetStateVectorFilePath() const = 0;
    };
}

#endif // SMP_ISTORAGEWRITER_H_
