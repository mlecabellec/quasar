// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IFactory.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IFACTORY_H_
#define SMP_IFACTORY_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/InvalidObjectName.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Uuid.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IComponent;
    class IComposite;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface for a component factory.
    class IFactory :
        public virtual Smp::IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IFactory() noexcept = default;

        /// Get Universally unique identifier of the type instantiated by the
        /// factory.
        /// @return  Universally unique identifier of component.
        virtual Smp::Uuid GetUuid() const = 0;

        /// Returns the fully qualified C++ name of the type.
        /// @return  Fully qualified C++ name of type that is created by this
        /// factory.
        virtual Smp::String8 GetTypeName() const = 0;

        /// Create a new instance with given name, description and parent.
        /// @param   name Name of the new instance.
        /// If the name provided is not a valid object name, an exception of
        /// type InvalidObjectName is raised.
        /// @param   description Description of the new instance.
        /// @param   parent Parent object of the new instance.
        /// @return  New component instance.
        /// @throws  Smp::InvalidObjectName
        virtual Smp::IComponent* CreateInstance(
            Smp::String8 name,
            Smp::String8 description,
            Smp::IComposite* parent) = 0;

        /// Delete an existing instance.
        /// @param   instance Instance to delete.
        virtual void DeleteInstance(
            Smp::IComponent* instance) = 0;
    };
}

#endif // SMP_IFACTORY_H_
