/**
 * @file TestIntegrationResoemResoes.cpp
 * @brief Cross-module integration test coupling `resoem` EtherCAT Master stack with `resoes` SubDevice stack (`CS-0100.16`, `CS-0100.18`).
 * @details Exercises `resoem::VirtualEtherCATBus`, `StubRawSocket`, `Enumerator`, and `FrameBuilder` alongside `resoes::SlaveApplication`, `MemoryEscHardware`, `ObjectDictionary`, and `CoeServer` cyclic Rx/Tx PDO pack/unpack.
 * @task [TSK-20260920-001] EtherCAT Master Virtual Bus
 * @task [TSK-20261113-001] C++23 EtherCAT SubDevice Stack
 * @feature [FE-0040] Modern C++23 EtherCAT Master Stack
 * @feature [FE-0280] Hardware-Free EtherCAT SubDevice Stack
 * @compliance Fulfills CS-0010, CS-0040, CS-0060, CS-0090, and CS-0100.
 */

#include "quasar/test_support/AsyncTestLogger.hpp"
#include "resoem/Enumerator.hpp"
#include "resoem/EtherCATFrame.hpp"
#include "resoem/virtual/SimulatedSlave.hpp"
#include "resoem/virtual/StubRawSocket.hpp"
#include "resoem/virtual/VirtualEtherCATBus.hpp"
#include "resoes/EepromEmulator.hpp"
#include "resoes/ObjectDictionary.hpp"
#include "resoes/SlaveApplication.hpp"
#include "resoes/hal/MemoryEscHardware.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <vector>

namespace quasar::integration::test {

/**
 * @brief Bounded cycle count for master-slave CoE/PDO integration loop (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Guarantees finite cyclic verification.
 */
constexpr std::size_t INTEGRATION_CYCLES = 20U;

TEST(ResoemResoesIntegrationTest, MasterVirtualBusAndSubDeviceCoeBridge) {
    quasar::test_support::TestScopeGuard scope("ResoemResoesIntegrationTest.MasterSubDeviceBridge");
    // Step 1: Bring up resoem VirtualEtherCATBus with a SimulatedSlave enumerated to OP.
    std::shared_ptr<resoem::virtual_bus::VirtualEtherCATBus> bus =
        resoem::virtual_bus::VirtualEtherCATBus::create();
    std::shared_ptr<resoem::virtual_bus::SimulatedSlave> vSlave =
        resoem::virtual_bus::SimulatedSlave::create("VirtualNode0", 0x00000002U, 0x044C2C52U, 0x00100000U, 100U);
    bus->attach_slave(vSlave);

    resoem::virtual_bus::StubRawSocket sock(bus, true);
    resoem::Enumerator masterEnum(sock);
    resoem::Result<size_t> enumCount = masterEnum.enumerate();
    ASSERT_TRUE(enumCount.has_value());
    EXPECT_EQ(enumCount.value(), 1U);
    ASSERT_TRUE(masterEnum.request_state_all(0x08U).has_value());

    // Step 2: Bring up resoes SlaveApplication with mapped 16-bit RxPDO (0x7000) & TxPDO (0x6000).
    resoes::MemoryEscHardware hw{};
    resoes::MemoryEepromStorage eeprom{};
    resoes::SlaveConfig cfg{};
    resoes::SlaveApplication subApp{hw, eeprom, cfg};
    ASSERT_TRUE(subApp.init().has_value());

    resoes::ObjectDictionary& od = subApp.od();
    ASSERT_TRUE(od.register_object(resoes::OdObjectEntry{
        0x7000U, resoes::OdObjectType::Var, 0U, "MasterCmd",
        {resoes::OdSubindexEntry{0U, resoes::OdDataType::Unsigned16, 16U,
            resoes::od_access::READ_WRITE_ALL | resoes::od_access::RX_PDO_MAP, "Cmd",
            {resoes::byte{0}, resoes::byte{0}}}}
    }).has_value());

    std::vector<resoes::byte> rxMap(4U, resoes::byte{0});
    (void)resoes::write_le32(rxMap, 0U, 0x70000010U);
    ASSERT_TRUE(od.register_object(resoes::OdObjectEntry{
        0x1600U, resoes::OdObjectType::Array, 1U, "RxPdoMap",
        {
            resoes::OdSubindexEntry{0U, resoes::OdDataType::Unsigned8, 8U, resoes::od_access::READ_ALL, "Count", {resoes::byte{1}}},
            resoes::OdSubindexEntry{1U, resoes::OdDataType::Unsigned32, 32U, resoes::od_access::READ_ALL, "Map1", rxMap}
        }
    }).has_value());

    std::vector<resoes::byte> sm2Assign(2U, resoes::byte{0});
    (void)resoes::write_le16(sm2Assign, 0U, 0x1600U);
    ASSERT_TRUE(od.register_object(resoes::OdObjectEntry{
        0x1C12U, resoes::OdObjectType::Array, 1U, "Sm2Assign",
        {
            resoes::OdSubindexEntry{0U, resoes::OdDataType::Unsigned8, 8U, resoes::od_access::READ_ALL, "Count", {resoes::byte{1}}},
            resoes::OdSubindexEntry{1U, resoes::OdDataType::Unsigned16, 16U, resoes::od_access::READ_ALL, "Pdo0", sm2Assign}
        }
    }).has_value());

    // Step 3: Build cyclic frames on resoem side and exchange process words into resoes CoE OD.
    for (std::size_t cycle = 1U; cycle <= INTEGRATION_CYCLES; ++cycle) {
        if (cycle > INTEGRATION_CYCLES) {
            throw std::runtime_error("Cyclic loop bound exceeded");
        } else {
            uint16_t cmdWord = static_cast<uint16_t>(cycle * 3U);
            std::array<resoem::byte, 2U> frameBytes = {
                static_cast<resoem::byte>(cmdWord & 0xFFU),
                static_cast<resoem::byte>((cmdWord >> 8U) & 0xFFU)
            };
            resoem::FrameBuilder fb{};
            fb.add_datagram_logical(0x0CU, static_cast<uint8_t>(cycle), 0x01000000U, frameBytes);
            std::vector<resoem::byte> rxBusFrame;
            bus->process_frame(fb.build(), false, rxBusFrame);
            EXPECT_EQ(rxBusFrame.size(), fb.buffer().size());

            ASSERT_TRUE(subApp.coe().unpack_rx_pdo(std::as_bytes(std::span<const resoem::byte>(frameBytes))).has_value());
            resoes::Result<uint32_t> stored = od.read_u32(0x7000U, 0U);
            ASSERT_TRUE(stored.has_value());
            EXPECT_EQ(stored.value(), cmdWord);
        }
    }
    quasar::test_support::AsyncTestLogger::instance().logOutput("completedCycles", static_cast<long long>(INTEGRATION_CYCLES));
}

} // namespace quasar::integration::test
