// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IProperty.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IPROPERTY_H_
#define SMP_IPROPERTY_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/AccessKind.h"
#include "Smp/AnySimple.h"
#include "Smp/InvalidAccess.h"
#include "Smp/InvalidPropertyValue.h"
#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/ViewKind.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{

    namespace Publication
    {
        class IType;
    }
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// This interface describes a published property.
    class IProperty :
        public virtual Smp::IObject
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IProperty() noexcept = default;

        /// Provides the type of the property.
        /// @return  Type of the property.
        virtual const Smp::Publication::IType* GetType() const = 0;

        /// Provides the access kind of the property.
        /// @return  Access kind of the property.
        virtual Smp::AccessKind GetAccess() const = 0;

        /// Provides the view kind of the property.
        /// @return  View kind of the property.
        virtual Smp::ViewKind GetView() const = 0;

        /// Provides the value of the property.
        /// Throws InvalidAccess if the property is Write Only.
        /// @return  The current value of the property.
        /// @throws  Smp::InvalidAccess
        virtual Smp::AnySimple GetValue() const = 0;

        /// Sets the value of the property.
        /// Throws InvalidAccess if the property is Read Only.
        /// Throws InvalidAnyType if the value is of wrong primitive type kind.
        /// @param   value New value of the property.
        /// @throws  Smp::InvalidAccess
        /// @throws  Smp::InvalidPropertyValue
        virtual void SetValue(
            Smp::AnySimple value) = 0;

        /// Get primitive type kind that this property uses.
        /// @return  Primitive type kind that this property uses.
        virtual Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const = 0;
    };
}

#endif // SMP_IPROPERTY_H_
