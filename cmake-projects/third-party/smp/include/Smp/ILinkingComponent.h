// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/ILinkingComponent.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ILINKINGCOMPONENT_H_
#define SMP_ILINKINGCOMPONENT_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/IComponent.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface for a component which can hold links to other components.
    /// Links include (but are not limited to) the following: <ul>
    /// <li>Interface links: An interface link is a link to an interface
    /// provided by another component.</li> <li>Event links: An event link is a
    /// link to an event sink of another component.</li> <li>Entry point links:
    /// An entry point link is a link to an entry point of another
    /// component.</li> </ul>
    /// This interface extends the component interface and adds a method which
    /// asks a component to remove all its links to another component. A link
    /// to another component is any form of dependency that requires the
    /// existence of another component.
    class ILinkingComponent :
        public virtual Smp::IComponent
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~ILinkingComponent() noexcept = default;

        /// Asks a component to remove all its links to the given target
        /// component.
        /// After this method has been called, the component must not try to
        /// access the given target component anymore.
        /// @param   target Target component to which all links shall be
        /// removed.
        virtual void RemoveLinks(
            const Smp::IComponent* target) = 0;
    };
}

#endif // SMP_ILINKINGCOMPONENT_H_
