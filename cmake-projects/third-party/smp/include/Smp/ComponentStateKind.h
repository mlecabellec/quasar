// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/ComponentStateKind.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_COMPONENTSTATEKIND_H_
#define SMP_COMPONENTSTATEKIND_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include <iosfwd>
#include "Smp/Int32.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This is an enumeration of the available states of a component. Each
    /// component is always in one of these four component states.
    enum class ComponentStateKind : Smp::Int32
    {
        /// The Created state is the initial state of a component. Component
        /// creation is done by an external mechanism, e.g. by factories. 
        /// This state is entered automatically after the component has been
        /// created. 
        /// This state is left via the Publish() state transition.
        CSK_Created,

        /// In Publishing state, the component is allowed to publish features.
        /// This includes publication of fields, operations and properties. In
        /// addition, the component is allowed to create other components. 
        /// This state is entered via the Publish() state transition. 
        /// This state is left via the Configure() state transition.
        CSK_Publishing,

        /// In Configured state, the component has been fully configured. This
        /// configuration may be done by external components, or internally by
        /// the component itself, e.g. by reading data from an external source. 
        /// This state is entered via the Configure() state transition. 
        /// This state is left via the Connect() state transition.
        CSK_Configured,

        /// In Connected state, the component is connected to the simulator. In
        /// this state, neither publication nor creation of other components is
        /// allowed anymore. Configuration performed via loading of SMDL
        /// configuration file and/or calling of initialisation entry point are
        /// performed in this state.
        /// This state is entered via the Connect() state transition.
        /// This state is left via the Disconnect() state transition or on
        /// simulation termination.
        CSK_Connected,

        /// In Disconnected state, the component is disconnected from the
        /// simulator, and all references to it are deleted, so that it can be
        /// deleted.
        /// This state is entered via the Disconnect() state transition.
        /// This is the final state of a component, and only left on deletion.
        CSK_Disconnected
    };

    /// Stream operator for ComponentStateKind to be able to print an enum value.
    /// @param os Output stream to output to.
    /// @param obj Object to output to stream.
    std::ostream& operator << (std::ostream& os, const ComponentStateKind& obj);
}

#endif // SMP_COMPONENTSTATEKIND_H_
