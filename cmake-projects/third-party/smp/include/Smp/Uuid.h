// ----------------------------------------------------------------------------
// Origin:    European Cooperation for Space Standardization (ECSS)
// Project:   E-ST-40-07 Simulation Modelling Platform (SMP)
// File:      Smp/Uuid.h
// Version:   Issue 1.1 from 31/03/2025
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2025 Telespazio Germany GmbH
// ----------------------------------------------------------------------------

#ifndef SMP_UUID_H_
#define SMP_UUID_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------------
// ----------------------------------------------------------------------------

#include <array>
#include <cstdint>
#include <iosfwd>

// ----------------------------------------------------------------------------
// ------------------------ Forward Declarations ------------------------------
// ----------------------------------------------------------------------------

namespace std
{
    template<typename T> struct hash;
}


// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------------
// ----------------------------------------------------------------------------

/// SMP standard types and interfaces.
namespace Smp
{
    /// Universally Unique Identifier.
    /// For a unique identification of types (and hence models), SMP uses 
    /// Universally Unique Identifiers with the format specified by the Open 
    /// Group (http://www.opengroup.org).
    struct Uuid final
    {
        /// 8 hex nibbles.
        uint32_t Data1;

        /// 3x4 hex nibbles.
        std::array<uint16_t, 3> Data2;

        /// 6x2 hex nibbles.
        std::array<uint8_t, 6> Data3;

        /// Constructor based on actual content
        /// @param data1 First 8 hex nibbles.
        /// @param data2 Next 12 hex nibbles.
        /// @param data3 Final 12 hex nibbles.
        constexpr Uuid(
            uint32_t data1,
            const std::array<uint16_t, 3>& data2,
            const std::array<uint8_t, 6>& data3) noexcept
            : Data1(data1), Data2(data2), Data3(data3)
        {
        }

        /// Constructor based on the format of the UUID string
        /// @param data1 First 8 hex nibbles.
        /// @param data2_0 Next 4 hex nibbles.
        /// @param data2_1 Next 4 hex nibbles.
        /// @param data2_2 Next 4 hex nibbles.
        /// @param data3 Final 12 hex nibbles.
        constexpr Uuid(
            uint32_t data1,
            uint16_t data2_0,
            uint16_t data2_1,
            uint16_t data2_2,
            uint64_t data3) noexcept
            : Uuid{data1,
                   {{data2_0, data2_1, data2_2}},
                   {{uint8_t(data3 >> (5 * 8)),
                     uint8_t(data3 >> (4 * 8)),
                     uint8_t(data3 >> (3 * 8)),
                     uint8_t(data3 >> (2 * 8)),
                     uint8_t(data3 >> (1 * 8)),
                     uint8_t(data3 >> (0 * 8))}}}
        {
        }

        /// Constructor based on the format used by SMP2
        /// @param data1 First 8 hex nibbles.
        /// @param data2_0 Next 4 hex nibbles.
        /// @param data2_1 Next 4 hex nibbles.
        /// @param data3_0 Next 2 hex nibbles as character.
        /// @param data3_1 Next 2 hex nibbles as character.
        /// @param data3_2 Next 2 hex nibbles as character.
        /// @param data3_3 Next 2 hex nibbles as character.
        /// @param data3_4 Next 2 hex nibbles as character.
        /// @param data3_5 Next 2 hex nibbles as character.
        /// @param data3_6 Next 2 hex nibbles as character.
        /// @param data3_7 Next 2 hex nibbles as character.
        constexpr Uuid(
            uint32_t data1,
            uint16_t data2_0,
            uint16_t data2_1,
            const std::array<char, 8>& data3) noexcept
            : Uuid{data1,
                   {{data2_0, data2_1, static_cast<uint16_t>((uint8_t(data3[0]) << 8) | uint8_t(data3[1]))}},
                   {{uint8_t(data3[2]),
                     uint8_t(data3[3]),
                     uint8_t(data3[4]),
                     uint8_t(data3[5]),
                     uint8_t(data3[6]),
                     uint8_t(data3[7])}}}
        {
        }

        /// Default constructor
        constexpr Uuid() noexcept
            : Uuid{0, 0, 0, 0, 0}
        {
        }

        /// Constructor based on string.
        /// This constructor cannot be used in constexpr declarations.
        /// @param value UUID represented as a string.
        /// @throw Smp::Exception if the string value cannot be parsed
        ///        into a UUID.
        explicit Uuid(
            const char* value);

        /// Default destructor
        ~Uuid() noexcept = default;

        /// Default copy constructor
        constexpr Uuid(const Smp::Uuid&) noexcept = default;

        /// Default move constructor
        constexpr Uuid(Smp::Uuid&&) noexcept = default;

        /// Default copy operator
        Uuid& operator=(const Smp::Uuid&) noexcept = default;

        /// Default move operator
        Uuid& operator=(Smp::Uuid&&) noexcept = default;

        /// Equality operator
        /// @param other Other instance to compare with.
        /// @return True if instances are equal, false otherwise.
        bool operator==(
            const Smp::Uuid& other) const;

        /// Inequality operator
        /// @param other Other instance to compare with.
        /// @return True if instances are not equal, false otherwise.
        bool operator!=(
            const Smp::Uuid& other) const;

        /// Less than operator
        /// @param other Other instance to compare with.
        /// @return True if instances is less than other, false otherwise.
        bool operator<(
            const Uuid& other) const;
    };
    
    /// Stream output operator 
    std::ostream& operator<<(std::ostream& ostream, const Smp::Uuid& uuid);
}

namespace std
{
    /// Specialisation of std::hash for UUIDs.
    /// Mandating that UUID is hashable ensures that it is possible
    /// to use it as key in unordered container.
    template <>
    struct hash<Smp::Uuid>
    {
        /// It calculates the hash of the Uuid argument.
        /// @param uuid the uuid for which the hash has to be calculated.
        /// @return Hash value for instance.
        size_t operator()(const Smp::Uuid& uuid) const;
    };
}

#endif // SMP_UUID_H_
