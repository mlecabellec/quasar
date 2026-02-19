// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Services/ILinkRegistry.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_SERVICES_ILINKREGISTRY_H_
#define SMP_SERVICES_ILINKREGISTRY_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/ComponentCollection.h"
#include "Smp/IService.h"
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
    /// Namespace for simulation services
    namespace Services
    {
        /// This interface is implemented by the Link Registry Service.
        /// The link registry maintains a global collection of link counters
        /// between components. Link counters can be incremented and
        /// decremented, and can be queried for. Further, the link registry
        /// supports fetching and removing all links to a given target.
        class ILinkRegistry :
            public virtual Smp::IService
        {
        public:
            /// Virtual destructor to release memory.
            virtual ~ILinkRegistry() noexcept = default;

            /// Add a link from source component to target component.
            /// This increments the link count between source and target.
            /// This method can be called several times with the same
            /// arguments, when a source component has several links to the
            /// same target component.
            /// This method informs the link registry that a link between two
            /// components has been created. The link registry does not create
            /// this link, it only gets told about its existence.
            /// @param   source Source component of link (i.e. the component
            /// that links to another component).
            /// @param   target Target component of link (i.e. the component
            /// that is being linked to by another component).
            virtual void AddLink(
                Smp::IComponent* source,
                const Smp::IComponent* target) = 0;

            /// Returns the number of links between source and target.
            /// This number is incremented each time AddLink() is called, and
            /// decremented each time that RemoveLink() is called.
            /// @param   source Source component of link (i.e. the component
            /// that links to another component).
            /// @param   target Target component of link (i.e. the component
            /// that is being linked to by another component).
            /// @return  True if such a link has been added before (and not
            /// been removed), false otherwise.
            virtual Smp::UInt32 GetLinkCount(
                const Smp::IComponent* source,
                const Smp::IComponent* target) const = 0;

            /// Remove a link between source and target that has been added to
            /// the service using AddLink() before.
            /// This decrements the link count between source and target.
            /// This method can be called several times with the same
            /// arguments, when a source component had several links to the
            /// same target component.
            /// This method informs the link registry that a link between two
            /// components has been removed. The link registry does not remove
            /// this link, it only gets told about the fact that the link no
            /// longer exists.
            /// @param   source Source component of link (i.e. the component
            /// that links to another component).
            /// @param   target Target component of link (i.e. the component
            /// that is being linked to by another component).
            /// @return  True if the link count between source and target had
            /// been positive and has been decremented, false if the link count
            /// had already been 0.
            virtual Smp::Bool RemoveLink(
                Smp::IComponent* source,
                const Smp::IComponent* target) = 0;

            /// Returns a collection of all sources that have a link to the
            /// given target.
            /// This method returns the collection of source components for
            /// which a link to the given target component has been added to
            /// the link registry.
            /// @param   target Target component to return link sources for.
            /// @return  Collection of source components which link to the
            /// given target.
            virtual const Smp::ComponentCollection* GetLinkSources(
                const Smp::IComponent* target) const = 0;

            /// Returns true if all sources linking to the given target can be
            /// asked to remove their link(s), false otherwise.
            /// This method checks whether all sources that have a link to the
            /// given target implement the optional interface
            /// ILinkingComponent. If so, they can be asked to remove their
            /// links. The method returns false if at least one source exists
            /// that does not implement the ILinkingComponent interface.
            /// @param   target Target component to check for links.
            /// @return  True if all links to the given target can be removed,
            /// false otherwise.
            virtual Smp::Bool CanRemove(
                const Smp::IComponent* target) = 0;

            /// Removes all links to the given target.
            /// This method calls the RemoveLinks() method of all source
            /// components that implement the optional ILinkingComponent
            /// interface, so it asks all link sources to remove their links to
            /// the given target.
            /// @param   target Target component of link (i.e. the component
            /// that is being linked to by other components).
            virtual void RemoveLinks(
                const Smp::IComponent* target) = 0;
        };
    }
}

#endif // SMP_SERVICES_ILINKREGISTRY_H_
