// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IComposite.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ICOMPOSITE_H_
#define SMP_ICOMPOSITE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/ContainerCollection.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IContainer;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface for a composite.
    /// An object with components as children implements this interface. Child
    /// components are held in named containers.
    /// @remarks This interface represents the Composition mechanism in the SMP
    /// Metamodel (via Containers). In UML 2.0, this is represented by
    /// composite aggregation.
    class IComposite :
        public virtual Smp::IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IComposite() noexcept = default;

        /// Query for the collection of all containers of the composite.
        /// The returned collection may be empty if no containers exist for the
        /// composite.
        /// @return  Collection of containers.
        virtual const Smp::ContainerCollection* GetContainers() const = 0;

        /// Query for a container of this composite by its name.
        /// The returned container may be nullptr if no container with the
        /// given name could be found.
        /// @param   name Container name.
        /// @return  Container queried for by name, or nulltr if no container
        /// with this name exists.
        virtual Smp::IContainer* GetContainer(
            Smp::String8 name) const = 0;
    };
}

#endif // SMP_ICOMPOSITE_H_
