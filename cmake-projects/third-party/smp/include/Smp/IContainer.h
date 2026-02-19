// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IContainer.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ICONTAINER_H_
#define SMP_ICONTAINER_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/CannotDelete.h"
#include "Smp/ComponentCollection.h"
#include "Smp/ContainerFull.h"
#include "Smp/DuplicateName.h"
#include "Smp/InvalidObjectType.h"
#include "Smp/InvalidParent.h"
#include "Smp/IObject.h"
#include "Smp/NotContained.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IComponent;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface for a container.
    /// A container allows querying for its children.
    /// Containers are used together with the IComposite interface for
    /// composition.
    class IContainer :
        public virtual Smp::IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IContainer() noexcept = default;

        /// Query for the collection of all components in the container. 
        /// The returned collection may be empty if no components exist for the
        /// container.
        /// @return  Collection of contained components.
        virtual const Smp::ComponentCollection* GetComponents() const = 0;

        /// Query for a component contained in the container by name.
        /// The returned component may be nullptr if no child with the given
        /// name could be found.
        /// @param   name Component name.
        /// @return  Child component, or nullptr if no child component with the
        /// given name exists.
        virtual Smp::IComponent* GetComponent(
            Smp::String8 name) const = 0;

        /// Add a contained component to the container.
        /// This method raises an exception of type ContainerFull if called for
        /// a full container, i.e. when the Count has reached the Upper limit.
        /// It raises an exception of type DuplicateName when trying to add a
        /// component with a name that is already contained in the container,
        /// as this would lead to duplicate names in the container. This method
        /// may raise an exception of type InvalidObjectType when it expects
        /// the given component to implement another interface as well.
        /// @param   component New contained component.
        /// @throws  Smp::ContainerFull
        /// @throws  Smp::DuplicateName
        /// @throws  Smp::InvalidObjectType
        /// @throws  Smp::InvalidParent
        virtual void AddComponent(
            Smp::IComponent* component) = 0;

        /// Delete a contained component from the container, and from memory.
        /// This method raises an exception of type NotContained if called for
        /// a component that has not been added to the container before. If the
        /// container contains the minimum number of components, the method
        /// throws a CannotDelete exception.
        /// @param   component Component to delete from container, and from
        /// memory.
        /// @throws  Smp::CannotDelete
        /// @throws  Smp::NotContained
        virtual void DeleteComponent(
            Smp::IComponent* component) = 0;

        /// Query for the number of components in the collection.
        /// @return  Current number of components in the collection.
        virtual Smp::Int64 GetCount() const = 0;

        /// Query the maximum number of components in the collection.
        /// A return value of -1 indicates that the collection has no upper
        /// limit.
        /// @remarks This is consistent with the use of upper bounds in UML,
        /// where a value of -1 represents no limit (typically shown as *).
        /// @remarks This information can be used to check whether another
        /// component can be added to the collection.
        /// @return  Maximum number of components in the collection (-1 =
        /// unlimited).
        virtual Smp::Int64 GetUpper() const = 0;

        /// Query the minimum number of components in the collection.
        /// @remarks This information can be used to validate a model
        /// hierarchy. If a collection specifies a Lower value above its
        /// current Count, then it is not properly configured. An external
        /// component may use this information to validate the configuration
        /// before executing it.
        /// @return  Minimum number of components in the collection or 0 when
        /// no minimum number has been defined.
        virtual Smp::Int64 GetLower() const = 0;
    };
}

#endif // SMP_ICONTAINER_H_
