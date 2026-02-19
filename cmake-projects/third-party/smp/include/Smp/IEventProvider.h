// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IEventProvider.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IEVENTPROVIDER_H_
#define SMP_IEVENTPROVIDER_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/EventSourceCollection.h"
#include "Smp/IComponent.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IEventSource;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface of an event provider.
    /// This is an optional interface. It needs to be implemented by components
    /// which want to allow access to event sources by name.
    /// An event provider is a component that holds event sources, which allow
    /// other components to subscribe their event sinks.
    class IEventProvider :
        public virtual Smp::IComponent
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IEventProvider() noexcept = default;

        /// Query for the collection of all event sources of the component.
        /// The collection may be empty if no event sources exist.
        /// @return  Collection of event sources.
        virtual const Smp::EventSourceCollection* GetEventSources() const = 0;

        /// Query for an event source of this component by its name.
        /// The returned event source may be nullptr if no event source with
        /// the given name could be found.
        /// @param   name Event source name.
        /// @return  Event source with the given name or nullptr if no event
        /// source with the given name could be found.
        virtual Smp::IEventSource* GetEventSource(
            Smp::String8 name) const = 0;
    };
}

#endif // SMP_IEVENTPROVIDER_H_
