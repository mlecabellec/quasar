/**
 * @file VirtualBusConstants.hpp
 * @brief Constants for simulated EtherCAT slave and virtual bus testing layer.
 * @details Contributes to [TSK-20260920-001.11] Hardware-Free Virtual Testing Layer.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace resoem::virtual_bus {

/// Total size of simulated ESC register memory (4096 bytes per ETG.1000).
constexpr std::size_t SIMULATED_ESC_RAM_SIZE = 4096;

/// Total capacity in 16-bit words of simulated SII EEPROM.
constexpr std::size_t SIMULATED_EEPROM_WORDS = 1024;

/// Default EtherCAT ESC controller type (0x11 = standard EtherCAT IP core).
constexpr std::uint8_t DEFAULT_ESC_TYPE = 0x11;

/// Default EtherCAT ESC revision (0x01).
constexpr std::uint8_t DEFAULT_ESC_REVISION = 0x01;

/// Default ESC build number (0x0001).
constexpr std::uint16_t DEFAULT_ESC_BUILD = 0x0001;

/// Number of FMMU units supported by the simulated ESC.
constexpr std::uint8_t DEFAULT_FMMU_COUNT = 8;

/// Number of SyncManager units supported by the simulated ESC.
constexpr std::uint8_t DEFAULT_SM_COUNT = 4;

/// Number of physical ports on the simulated slave (Port 0: In, Port 1: Out).
constexpr std::uint8_t DEFAULT_PORT_COUNT = 2;

/// ESC features bitmask (0x0004 = Distributed Clocks 64-bit supported).
constexpr std::uint16_t DEFAULT_ESC_FEATURES = 0x0004;

/// Default base configured station address (0x1000).
constexpr std::uint16_t DEFAULT_BASE_STATION_ADDR = 0x1000;

/// Default propagation delay per slave in nanoseconds for DC testing.
constexpr std::uint32_t DEFAULT_PROPAGATION_DELAY_NS = 120;

/// Bitmask for DL Status Port 0 physical link present.
constexpr std::uint16_t DL_STATUS_PORT0_LINK = 0x0001;

/// Bitmask for DL Status Port 0 loop closed.
constexpr std::uint16_t DL_STATUS_PORT0_LOOP_CLOSED = 0x0002;

/// Bitmask for DL Status Port 1 physical link present.
constexpr std::uint16_t DL_STATUS_PORT1_LINK = 0x0004;

/// Bitmask for DL Status Port 1 loop closed.
constexpr std::uint16_t DL_STATUS_PORT1_LOOP_CLOSED = 0x0008;

/// Bitmask for DL Status Port 0 communication established.
constexpr std::uint16_t DL_STATUS_COMM_PORT0 = 0x0100;

/// Bitmask for DL Status Port 1 communication established.
constexpr std::uint16_t DL_STATUS_COMM_PORT1 = 0x0200;

/// Working counter increment value for successful read operation.
constexpr std::uint16_t WKC_INC_READ = 1;

/// Working counter increment value for successful write operation.
constexpr std::uint16_t WKC_INC_WRITE = 1;

/// Working counter increment value for successful read and write operation.
constexpr std::uint16_t WKC_INC_READWRITE = 3;

/// Maximum number of simulated slaves supported on a single virtual bus.
constexpr std::size_t MAX_SIMULATED_SLAVES = 512;

/// Maximum frame queue capacity for StubRawSocket.
constexpr std::size_t MAX_QUEUE_CAPACITY = 256;

/// Default timeout in milliseconds for virtual socket operations.
constexpr int DEFAULT_VIRTUAL_SOCKET_TIMEOUT_MS = 100;

} // namespace resoem::virtual_bus
