// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IDynamicInvocation.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IDYNAMICINVOCATION_H_
#define SMP_IDYNAMICINVOCATION_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/IComponent.h"
#include "Smp/InvalidOperationName.h"
#include "Smp/InvalidParameterCount.h"
#include "Smp/InvalidParameterValue.h"
#include "Smp/OperationCollection.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/PropertyCollection.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IOperation;
    class IProperty;
    class IRequest;
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface for a component that supports dynamic invocation of
    /// operations.
    /// A component may implement this interface in order to allow dynamic
    /// invocation of its operations.
    /// @remarks Dynamic invocation is typically used for scripting.
    class IDynamicInvocation :
        public virtual Smp::IComponent
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IDynamicInvocation() noexcept = default;

        /// Dynamically invoke an operation using a request object that has
        /// been created and filled with parameter values by the caller.
        /// This method raises the InvalidOperationName exception if the
        /// request object passed does not name an operation that allows
        /// dynamic invocation. When calling invoke with a wrong number of
        /// parameters, the InvalidParameterCount exception is raised. When
        /// passing a parameter of wrong type, the InvalidParameterValue
        /// exception is raised.
        /// @remarks The same request object can be used to invoke a method
        /// several times.
        /// @param   request Request object to invoke.
        /// @throws  Smp::InvalidParameterCount
        /// @throws  Smp::InvalidOperationName
        /// @throws  Smp::InvalidParameterValue
        virtual void Invoke(
            Smp::IRequest* request) = 0;

        /// Get the property of given name.
        /// This method returns nullptr if called with a property name for
        /// which no corresponding property exists.
        /// @param   name Property name.
        /// @return  Instance of property for given name, or nullptr if no
        /// property exists with the given name.
        virtual Smp::IProperty* GetProperty(
            Smp::String8 name) const = 0;

        /// Provides the collection of properties that have been published.
        /// @return  Collection of properties that have been published, which
        /// may be empty.
        virtual const Smp::PropertyCollection* GetProperties() const = 0;

        /// Get the operation of given name.
        /// This method returns nullptr if called with an operation name for
        /// which no corresponding operation exists.
        /// @param   name Operation name.
        /// @return  Instance of operation for given name, or nullptr if no
        /// operation exists with the given name.
        virtual Smp::IOperation* GetOperation(
            Smp::String8 name) const = 0;

        /// Provides the collection of operations that have been published.
        /// @return  Collection of operations that have been published, which
        /// may be empty.
        virtual const Smp::OperationCollection* GetOperations() const = 0;
    };
}

#endif // SMP_IDYNAMICINVOCATION_H_
