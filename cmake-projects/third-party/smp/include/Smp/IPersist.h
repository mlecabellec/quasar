// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IPersist.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IPERSIST_H_
#define SMP_IPERSIST_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/CannotRestore.h"
#include "Smp/CannotStore.h"
#include "Smp/IObject.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IStorageReader;
    class IStorageWriter;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface of a self-persisting object that provides operations to allow
    /// for storing and restoring its state.
    /// An object may implement this interface if it wants to have control over
    /// storing and restoring of its state. This is an optional interface which
    /// needs to be implemented by objects with self-persistence only.
    class IPersist :
        public virtual Smp::IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IPersist() noexcept = default;

        /// Restore object state from storage.
        /// This method raises an exception of type CannotRestore if reading
        /// data from the storage reader fails.
        /// @param   reader Interface that allows reading from storage.
        /// @throws  Smp::CannotRestore
        virtual void Restore(
            Smp::IStorageReader* reader) = 0;

        /// Store object state to storage.
        /// This method raises an exception of type CannotStore if writing data
        /// to the storage writer fails.
        /// @param   writer Interface that allows writing to storage.
        /// @throws  Smp::CannotStore
        virtual void Store(
            Smp::IStorageWriter* writer) = 0;
    };
}

#endif // SMP_IPERSIST_H_
