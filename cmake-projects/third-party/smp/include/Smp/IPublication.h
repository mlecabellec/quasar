// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/IPublication.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_IPUBLICATION_H_
#define SMP_IPUBLICATION_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/AccessKind.h"
#include "Smp/DuplicateName.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/InvalidType.h"
#include "Smp/NoDynamicInvocation.h"
#include "Smp/OperationCollection.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/PropertyCollection.h"
#include "Smp/Publication/IPublishField.h"
#include "Smp/Publication/TypeNotRegistered.h"
#include "Smp/Uuid.h"
#include "Smp/ViewKind.h"

// ----------------------------------------------------------------------------
// ---------------------------- Forward declarations --------------------------
// ----------------------------------------------------------------------------

namespace Smp
{
    class IOperation;
    class IProperty;

    namespace Publication
    {
        class IPublishOperation;
        class ITypeRegistry;
    }
}

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Interface that provides functionality to allow publishing members,
    /// including fields, properties and operations.
    class IPublication :
        public virtual Smp::Publication::IPublishField
    {
    public:
        /// Virtual destructor to release memory.
        virtual ~IPublication() noexcept = default;

        /// Publish an operation with the given name, description and view kind.
        /// If an Operation with the same name has not been published, this
        /// creates a new IPublishOperation instance and returns it.
        /// If an Operations with the same name has already been published,
        /// this will update the “description” and “view”  of the previous
        /// publication, clears all published Parameters for the Operation, and
        /// returns the same IPublishOperation of the previously published
        /// Operation.
        /// If an object with the same name has been published before, and it
        /// is not an Operation, an exception of type DuplicateName is thrown.
        /// It the component being published does not implement
        /// IDynamicInvocation, an exception of type NoDynamicInvocation is
        /// thrown.
        /// The operation parameters (including an optional return parameter)
        /// may be published against the returned IPublishOperation interface
        /// after calling PublishOperation().
        /// @param   name Name of operation.
        /// @param   description Description of operation.
        /// @param   view View kind of operation node in simulation tree.
        /// @return  Reference to the operation that has been created, to
        /// publish parameters against.
        /// @throws  Smp::InvalidObjectName
        /// @throws  Smp::DuplicateName
        /// @throws  Smp::NoDynamicInvocation
        virtual Smp::Publication::IPublishOperation* PublishOperation(
            Smp::String8 name,
            Smp::String8 description,
            Smp::ViewKind view = Smp::ViewKind::VK_All) = 0;

        /// Publish an operation defined internally that implements the
        /// IOperation interface.
        /// If an Operations with the same name has already been published,
        /// this will replace that operation with this one.
        /// If an object with the same name has been published before, and it
        /// is not an Operation, an exception of type DuplicateName is thrown.
        /// It the component being published does not implement
        /// IDynamicInvocation, an exception of type NoDynamicInvocation is
        /// thrown.
        /// @param   operation Operation to publish.
        /// @throws  Smp::DuplicateName
        /// @throws  Smp::NoDynamicInvocation
        virtual void PublishOperation(
            Smp::IOperation* operation) = 0;

        /// Publish a property with the given name, description and view kind.
        /// If a Property with the same name has not been published, this
        /// creates a new IProperty instance and returns it.
        /// If a Property with the same name has already been published, this
        /// will update the “description” , “view”, "access", and "type"  of
        /// the previous publication, and returns the same IProperty of the
        /// previously published Property.
        /// If an object with the same name has been published before, and it
        /// is not a Property, an exception of type DuplicateName is thrown.
        /// It the component being published does not implement
        /// IDynamicInvocation, an exception of type NoDynamicInvocation is
        /// thrown.
        /// It the primitive type kind of the publication type is None, it
        /// throws an exception of type InvalidType.
        /// @param   name Property name.
        /// @param   description Property description.
        /// @param   typeUuid Uuid of type of property.
        /// @param   accessKind Access kind of Property.
        /// @param   view Show field in model tree.
        /// @return  Reference to the property that has been generated by the
        /// publication call.
        /// @throws  Smp::Publication::TypeNotRegistered
        /// @throws  Smp::InvalidObjectName
        /// @throws  Smp::DuplicateName
        /// @throws  Smp::NoDynamicInvocation
        /// @throws  Smp::InvalidType
        virtual Smp::IProperty* PublishProperty(
            Smp::String8 name,
            Smp::String8 description,
            Smp::Uuid typeUuid,
            Smp::AccessKind accessKind,
            Smp::ViewKind view = Smp::ViewKind::VK_All) = 0;

        /// Publish a property defined internally that implements the IProperty
        /// interface.
        /// If a Property with the same name has already been published, this
        /// will replace that property with this one.
        /// If an object with the same name has been published before, and it
        /// is not a Property, an exception of type DuplicateName is thrown.
        /// It the component being published does not implement
        /// IDynamicInvocation, an exception of type NoDynamicInvocation is
        /// thrown.
        /// @param   property Property to publish.
        /// @throws  Smp::DuplicateName
        /// @throws  Smp::NoDynamicInvocation
        virtual void PublishProperty(
            Smp::IProperty* property) = 0;

        /// Get the property of given name.
        /// This method returns nullptr if called with a property name for
        /// which no corresponding property exists.
        /// @param   name Property name.
        /// @return  Instance of property for given name, or nullptr if no
        /// property exists with the given name.
        virtual Smp::IProperty* GetProperty(
            Smp::String8 name) const = 0;

        /// Provides the collection of properties that have been published.
        /// @remarks The collection shall exactly contain the properties that
        /// have been published via the PublishProperty method.
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
        /// @remarks The collection shall exactly contain the operations that
        /// have been published via the PublishOperation method.
        /// @return  Collection of operations that have been published, which
        /// may be empty.
        virtual const Smp::OperationCollection* GetOperations() const = 0;

        /// Give access to the global type registry.
        /// The type registry is typically a singleton, and must not be null,
        /// to allow use of existing types, and registration of new types.
        /// @return  Reference to global type registry.
        virtual Smp::Publication::ITypeRegistry* GetTypeRegistry() const = 0;

        /// Call this operation to release all data created during earlier
        /// Publish calls to this instance.
        /// This invalidated all pointers retrieved earlier via this instance.
        virtual void Unpublish() = 0;
    };
}

#endif // SMP_IPUBLICATION_H_
