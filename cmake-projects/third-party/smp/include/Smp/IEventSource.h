// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IEventSource.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IEVENTSOURCE_H_
#define SMP_IEVENTSOURCE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/EventSinkAlreadySubscribed.h"
#include "Smp/EventSinkCollection.h"
#include "Smp/EventSinkNotSubscribed.h"
#include "Smp/InvalidEventSink.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IEventSink;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface of an event source that event sinks (IEventSink) can
    /// subscribe to.
    /// This interface allows event consumers to subscribe to or unsubscribe
    /// from an event.
    class IEventSource :
        public virtual Smp::IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IEventSource() noexcept = default;

        /// Subscribe to the event source, i.e. request notifications.
        /// An event sink can only be subscribed once to each event source.
        /// Event sinks will be called in the order they have been subscribed
        /// to the event source.
        /// If the given event sink is already subscribed to the event source,
        /// an exception of type EventSinkAlreadySubscribed is thrown. If the
        /// type of the event argument of the event sink is not the type the
        /// event source expects, an exception of type InvalidEventSink is
        /// thrown.
        /// @param   eventSink Event sink to subscribe to event source.
        /// @throws  Smp::EventSinkAlreadySubscribed
        /// @throws  Smp::InvalidEventSink
        virtual void Subscribe(
            Smp::IEventSink* eventSink) = 0;

        /// Unsubscribe from the event source, i.e. cancel notifications.
        /// An event sink can only be unsubscribed if it has been subscribed
        /// before.
        /// This method raises the EventSinkNotSubscribed exception if the
        /// given event sink is not subscribed to the event source.
        /// @param   eventSink Event sink to unsubscribe from event source.
        /// @throws  Smp::EventSinkNotSubscribed
        virtual void Unsubscribe(
            Smp::IEventSink* eventSink) = 0;

        /// Get the primitive type kind of the event argument.
        /// Use PTK_None for an event without an argument.
        /// This operation allows for type checking between an Event Source
        /// (implementing IEventSource) and an event sink (implementing
        /// IEventSink) during Subscribe.
        /// @return  Primitive type kind of the event argument, or PTK_None for
        /// none.
        virtual Smp::PrimitiveTypeKind GetEventArgType() const = 0;

        /// This operation returns the collection of event sinks that have been
        /// subscribed to the event source.
        /// @return  The collection of event sinks that have been subscribed to
        /// the event source.
        virtual const Smp::EventSinkCollection* GetEventSinks() const = 0;
    };
}

#endif // SMP_IEVENTSOURCE_H_
