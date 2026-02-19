// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/SimulatorStateKind.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_SIMULATORSTATEKIND_H_
#define SMP_SIMULATORSTATEKIND_H_

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
    /// This is an enumeration of the available states of the simulator. The
    /// Setup phase is split into three different states, the Execution phase
    /// has five different states, and the Termination phase has two states.
    enum class SimulatorStateKind : Smp::Int32
    {
        /// In Building state, the component hierarchy is created. This is done
        /// by an external component, not by the simulator. 
        /// This state is entered automatically after the simulation
        /// environment has performed its initialisation.
        /// In this state, Publish() and Configure() can be called any time to
        /// call the corresponding Publish() and Configure() operations of each
        /// component.
        /// This state is left with the Connect() state transition method.
        SSK_Building,

        /// In Connecting state, the simulation environment traverses the
        /// component hierarchy and calls the Connect() method of each
        /// component.
        /// This state is entered using the Connect() state transition.
        /// After connecting all components to the simulator, an automatic
        /// state transition to the Initialising state is performed. 
        SSK_Connecting,

        /// In Initialising state, the simulation environment executes all
        /// initialisation entry points in the order they have been added to
        /// the simulator using the AddInitEntryPoint() method.
        /// This state is either entered automatically after the simulation
        /// environment has connected all models to the simulator, or manually
        /// from Standby state using the Initialise() state transition.
        /// After calling all initialisation entry points, an automatic state
        /// transition to the Standby state is performed. 
        SSK_Initialising,

        /// In Standby state, the simulation environment (namely the Time
        /// Keeper Service) does not progress simulation time. Only entry
        /// points registered relative to Zulu time are executed.
        /// This state is entered automatically from the Initialising, Storing,
        /// and Restoring states, or manually from the Executing state using
        /// the Hold() state transition.
        /// This state is left with one of the Run(), Store(), Restore(),
        /// Initialise(), Reconnect() or Exit() state transitions.
        SSK_Standby,

        /// In Executing state, the simulation environment (namely the Time
        /// Keeper Service) does progress simulation time. Entry points
        /// registered with any of the available time kinds are executed.
        /// This state is entered using the Run() state transition.
        /// This state is left using the Hold() state transition.
        SSK_Executing,

        /// In Storing state, the simulation environment first stores the
        /// values of all fields published with the State attribute to storage
        /// (typically a file). Afterwards, the Store() method of all
        /// components (Models and Services) implementing the optional IPersist
        /// interface is called, to allow custom storing of additional
        /// information. While in this state, fields published with the State
        /// attribute must not be modified by the models, to ensure that a
        /// consistent set of field values is stored. 
        /// This state is entered using the Store() state transition.
        /// After storing the simulator state, an automatic state transition to
        /// the Standby state is performed. 
        SSK_Storing,

        /// In Restoring state, the simulation environment first restores the
        /// values of all fields published with the State attribute from
        /// storage. Afterwards, the Restore() method of all components
        /// implementing the optional IPersist interface is called, to allow
        /// custom restoring of additional information. While in this state,
        /// fields published with the State attribute must not be modified by
        /// the models, to ensure that a consistent set of field values is
        /// restored. 
        /// This state is entered using the Restore() state transition.
        /// After restoring the simulator state, an automatic state transition
        /// to the Standby state is performed. 
        SSK_Restoring,

        /// In Reconnecting state, the simulation environment makes sure that
        /// models that have been added to the simulator after leaving the
        /// Building state are properly published, configured and connected.
        /// This state is entered using the Reconnect() state transition.
        /// After connecting all new models, an automatic state transition to
        /// the Standby state is performed.
        SSK_Reconnecting,

        /// In Exiting state, the simulation environment is properly
        /// terminating a running simulation.
        /// This state is entered using the Exit() state transition. After
        /// exiting, the simulator is in an undefined state. 
        SSK_Exiting,

        /// In this state, the simulation environment performs an abnormal
        /// simulation shut-down.
        /// This state is entered using the Abort() state transition. After
        /// aborting, the simulator is in an undefined state. 
        SSK_Aborting
    };

    /// Stream operator for SimulatorStateKind to be able to print an enum value.
    /// @param os Output stream to output to.
    /// @param obj Object to output to stream.
    std::ostream& operator << (std::ostream& os, const SimulatorStateKind& obj);
}

#endif // SMP_SIMULATORSTATEKIND_H_
