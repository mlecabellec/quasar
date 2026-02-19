// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/InvalidEventSink.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_INVALIDEVENTSINK_H_
#define SMP_INVALIDEVENTSINK_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/Exception.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IEventSink;
    class IEventSource;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This exception is raised when trying to subscribe an event sink to an
    /// event source that has a different event type.
    class InvalidEventSink : public virtual Smp::Exception
    {
    public:

        /// Virtual destructor to release memory.
        virtual ~InvalidEventSink() noexcept = default;

        /// Returns the event source that cannot be subscribed to the event
        /// sink.
        /// @return  Event source that cannot be subscribed to the event sink.
        virtual const Smp::IEventSource* GetEventSource() const noexcept = 0;

        /// Returns the event sink that the event source that cannot be
        /// subscribed to.
        /// @return  Event sink that the event source that cannot be subscribed
        /// to.
        virtual const Smp::IEventSink* GetEventSink() const noexcept = 0;
    };
}

#endif // SMP_INVALIDEVENTSINK_H_
