// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/AnySimple.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_ANYSIMPLE_H_
#define SMP_ANYSIMPLE_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include "Smp/PrimitiveTypes.h"
#include <iosfwd>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Variant that can store a value of any of the simple types. The type 
    /// attribute defines the type used to represent the value, while the value
    /// attribute contains the actual value.
    class AnySimple final
    {
    public:
        /// Initialises with PTK_None
        AnySimple();

        /// Initialises with kind and no content
        /// @param kind Primitive type kind to set in new instance.
        explicit AnySimple(
            Smp::PrimitiveTypeKind kind);

        /// Inialises using one of the setters
        template <typename Type>
        AnySimple(
            Smp::PrimitiveTypeKind kind, 
            Type value) : AnySimple(kind)
        {
            this->SetValue(kind, value);
        }

        /// Copy Constructor
        /// @param other Other instance to copy from.
        AnySimple(
            const Smp::AnySimple& other);

        /// Move Constructor
        /// @param other Other instance to move from.
        AnySimple(
            Smp::AnySimple&& other) noexcept;

        /// Assignment operator
        /// @param other Other instance to assign from.
        /// @return New instance with identical content.
        AnySimple& operator=(
            const Smp::AnySimple& other);

        /// Move operator
        /// @param other Other instance to move from.
        /// @return New instance with identical content.
        AnySimple& operator=(
            Smp::AnySimple&& other) noexcept;

        /// Destructor
        ~AnySimple() noexcept;

        /// Set value from Bool.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::Bool value);

        /// Set value from Char8.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::Char8 value);

        /// Set value from String8.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::String8 value);

        /// Set value from UInt8.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::UInt8 value);

        /// Set value from UInt16.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::UInt16 value);

        /// Set value from UInt32.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::UInt32 value);

        /// Set value from UInt64.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::UInt64 value);

        /// Set value from Int8.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::Int8 value);

        /// Set value from Int16.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::Int16 value);

        /// Set value from Int32.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::Int32 value);

        /// Set value from Int64.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::Int64 value);

        /// Set value from Float32.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::Float32 value);

        /// Set value from Float64.
        /// @param kind Primitive type to use for internal storage.
        /// @param value Value to store into instance.
        void SetValue(
            Smp::PrimitiveTypeKind kind,
            Smp::Float64 value);

        /// Return value as Bool.
        operator Smp::Bool() const;

        /// Return value as Char8.
        operator Smp::Char8() const;

        /// Return value as String8.
        operator Smp::String8() const;

        /// Return value as UInt8.
        operator Smp::UInt8() const;

        /// Return value as UInt16.
        operator Smp::UInt16() const;

        /// Return value as UInt32.
        operator Smp::UInt32() const;

        /// Return value as UInt64.
        operator Smp::UInt64() const;

        /// Return value as Int8.
        operator Smp::Int8() const;

        /// Return value as Int16.
        operator Smp::Int16() const;

        /// Return value as Int32.
        operator Smp::Int32() const;

        /// Return value as Int64.
        operator Smp::Int64() const;

        /// Return value as Float32.
        operator Smp::Float32() const;

        /// Return value as Float64.
        operator Smp::Float64() const;

        /// Transfer the string stored in the instance to the caller
        /// @return String that was owned by instance before.
        Smp::String8 MoveString();

        /// Get the primitive type stored within this instance.
        /// @return Primitive type stored within this instance.
        Smp::PrimitiveTypeKind GetType() const noexcept;

        /// Equality operator
        /// @param other Other instance to compare with.
        /// @return True if instances are equal, false otherwise.
        bool operator==(
            const Smp::AnySimple& other) const;

        /// Inequality operator
        /// @param other Other instance to compare with.
        /// @return True if instances are not equal, false otherwise.
        bool operator!=(
            const Smp::AnySimple& other) const;

        /// Union of primitive type values, which is used for the value field
        /// of AnySimple.
        union PrimitiveTypeValue
        {
            /// 8 bit character value.
            Smp::Char8 char8Value;

            /// Boolean with true and false.
            Smp::Bool boolValue;

            /// 8 bit signed integer value.
            Smp::Int8 int8Value;

            /// 8 bit unsigned integer value.
            Smp::UInt8 uInt8Value;

            /// 16 bit signed integer value.
            Smp::Int16 int16Value;

            /// 16 bit unsigned integer value.
            Smp::UInt16 uInt16Value;

            /// 32 bit signed integer value.
            Smp::Int32 int32Value;

            /// 32 bit unsigned integer value.
            Smp::UInt32 uInt32Value;

            /// 64 bit signed integer value.
            Smp::Int64 int64Value;

            /// 64 bit unsigned integer value.
            Smp::UInt64 uInt64Value;

            /// 32 bit single-precision floating-point value.
            Smp::Float32 float32Value;

            /// 64 bit double-precision floating-point value.
            Smp::Float64 float64Value;

            /// Duration in nanoseconds.
            Smp::Duration durationValue;

            /// Absolute time in nanoseconds.
            Smp::DateTime dateTimeValue;

            /// 8 bit character string value.
            Smp::String8 string8Value;
        };

        /// Type of the contained value.
        Smp::PrimitiveTypeKind type;

        /// Value stored in the AnySimple instance.
        Smp::AnySimple::PrimitiveTypeValue value;
    };

    /// Stream operator for AnySimple to be able to print an AnySimple.
    /// @param os Output stream to output to.
    /// @param obj Object to output to stream.
    std::ostream& operator << (std::ostream& os, const AnySimple& obj);
}

#endif // SMP_ANYSIMPLE_H_
