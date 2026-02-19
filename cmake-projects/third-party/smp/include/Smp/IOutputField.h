// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IOutputField.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IOUTPUTFIELD_H_
#define SMP_IOUTPUTFIELD_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/FieldAlreadyConnected.h"
#include "Smp/FieldCollection.h"
#include "Smp/FieldNotConnected.h"
#include "Smp/IField.h"
#include "Smp/InvalidTarget.h"
#include "Smp/PrimitiveTypes.h"

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This interface is implemented by a Field that can take part as output
    /// field in direct inter-component data flow.
    class IOutputField :
        public virtual Smp::IField
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IOutputField() noexcept = default;

        /// Connect this field to a target field for direct data flow.
        /// As the Push() operation only requires to set a value, the target
        /// field can be any field (it does not need to implement a specific
        /// interface for an input field).
        /// @param   target Target field to connect to. The field type must be
        /// compatible.
        /// @throws  Smp::InvalidTarget
        /// @throws  Smp::FieldAlreadyConnected
        virtual void Connect(
            Smp::IField* target) = 0;

        /// Disconnect a target field (that has been connected before) from
        /// this field.
        /// @param   target Target field to disconnect. The field must have
        /// been connected before.
        /// @throws  Smp::FieldNotConnected
        virtual void Disconnect(
            Smp::IField* target) = 0;

        /// Push the current field value to all connected target fields.
        virtual void Push() = 0;

        /// Collection of fields that have been connected to the output field
        /// for data flow.
        /// @return  Collection of fields that have been connected to the ouput
        /// field.
        virtual const Smp::FieldCollection* GetInputFields() const = 0;

        /// Flag to check whether the output field supports automatic data
        /// propagation (i.e. automatic Push() each time the field value is
        /// changed) or not.
        /// @return  True if the output field supports automatic data
        /// propagation, false otherwise.
        virtual Smp::Bool IsAutomatic() const = 0;
    };
}

#endif // SMP_IOUTPUTFIELD_H_
