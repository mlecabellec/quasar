// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IEventSink.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IEVENTSINK_H_
#define SMP_IEVENTSINK_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/AnySimple.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface of an event sink that can be subscribed to an event source
    /// (IEventSource).
    /// This interface provides a notification method (event handler) that can
    /// be called by event sources when an event is emitted.
    class IEventSink :
        public virtual Smp::IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IEventSink() noexcept = default;

        /// Get the primitive type kind of the event argument.
        /// Use PTK_None for an event without an argument.
        /// This operation allows for type checking between an Event Source
        /// (implementing IEventSource) and an event sink (implementing
        /// IEventSink) during Subscribe.
        /// @return  Primitive type kind of the event argument, or PTK_None for
        /// none.
        virtual Smp::PrimitiveTypeKind GetEventArgType() const = 0;

        /// This event handler method is called when an event is emitted.
        /// Components providing event sinks must ensure that these event sinks
        /// do not throw exceptions.
        /// @param   sender Object emitting the event.
        /// @param   arg Event argument with context data for event
        /// notification.
        /// The type of the event argument (PTK_None or one of the existing
        /// Primitive Type Kinds) depends on the type of the event sink.
        virtual void Notify(
            Smp::IObject* sender,
            Smp::AnySimple arg) = 0;
    };
}

#endif // SMP_IEVENTSINK_H_
