/**
 * @file TestResoemExtended.cpp
 * @brief Extended CS-0100 test suite for `resoem` virtual EtherCAT bus, frame builder, ENI parser, fault injection, and concurrency.
 * @details Uses `VirtualEtherCATBus` and `StubRawSocket` so all tests run hermetically without root privileges or physical `eth0` interfaces.
 * @task [TSK-20260920-001] EtherCAT Cable Redundancy, Hot Connect & ENI Framework
 * @feature [FE-0040] Modern C++23 EtherCAT Master Stack
 * @compliance Fulfills CS-0010, CS-0040, CS-0060, CS-0090, and CS-0100.
 */

#include "resoem/EniDocument.hpp"
#include "resoem/Enumerator.hpp"
#include "resoem/EtherCATFrame.hpp"
#include "resoem/EthercatMasterService.hpp"
#include "resoem/virtual/SimulatedSlave.hpp"
#include "resoem/virtual/StubRawSocket.hpp"
#include "resoem/virtual/VirtualEtherCATBus.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace resoem::test {

/**
 * @brief Frame fuzz iteration cap (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Prevents unbounded fuzzing.
 */
constexpr std::size_t MAX_FRAME_FUZZ_ITERS = 64U;

/**
 * @brief Worker threads for virtual bus stress (`CS-0040.2`).
 * @requirement [FE-0040.8] Concurrent Virtual Bus Safety
 * @compliance Bounded worker count.
 */
constexpr std::size_t BUS_STRESS_WORKERS = 4U;

/**
 * @brief Frame transactions per virtual bus worker (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Guarantees deterministic execution duration.
 */
constexpr std::size_t FRAMES_PER_WORKER = 50U;

/**
 * @brief Fixed PRNG seed (`CS-0100.12`).
 * @requirement [CS-0100.12] Deterministic Testing
 * @compliance Reproducible datagram payloads.
 */
constexpr std::uint64_t RESOEM_FUZZ_SEED = 0xE74EC470ULL;

TEST(ResoemTutorialTest, Tutorial1_FrameBuilderMultiDatagramConstruction) {
    quasar::test_support::TestScopeGuard scope("ResoemTutorialTest.Tutorial1");
    // Step 1: Initialize FrameBuilder and add physical APRD + logical LRW datagrams.
    FrameBuilder builder{};
    std::array<byte, 4U> aprdPayload = {byte{0}, byte{0}, byte{0}, byte{0}};
    std::array<byte, 2U> lrwPayload = {static_cast<byte>(0x12U), static_cast<byte>(0x34U)};
    builder.add_datagram(0x01U, 0U, 0x0000U, 0x0130U, aprdPayload);
    builder.add_datagram_logical(0x0CU, 1U, 0x01000000U, lrwPayload);

    // Step 2: Finalize frame bytes and verify non-empty buffer.
    std::span<const byte> built = builder.build();
    EXPECT_GT(built.size(), 14U);
    EXPECT_EQ(built.size(), builder.buffer().size());
    builder.reset();
    quasar::test_support::AsyncTestLogger::instance().logOutput("builtFrameSize", static_cast<long long>(built.size()));
}

TEST(ResoemTutorialTest, Tutorial2_VirtualBusEnumerationAndStateTransitions) {
    quasar::test_support::TestScopeGuard scope("ResoemTutorialTest.Tutorial2");
    // Step 1: Create VirtualEtherCATBus with two SimulatedSlave devices.
    std::shared_ptr<virtual_bus::VirtualEtherCATBus> bus =
        virtual_bus::VirtualEtherCATBus::create();
    std::shared_ptr<virtual_bus::SimulatedSlave> slave0 =
        virtual_bus::SimulatedSlave::create("Drive0", 0x00000002U, 0x044C2C52U, 0x00100000U, 1U);
    std::shared_ptr<virtual_bus::SimulatedSlave> slave1 =
        virtual_bus::SimulatedSlave::create("IO1", 0x00000002U, 0x03E93052U, 0x00100000U, 2U);
    bus->attach_slave(slave0);
    bus->attach_slave(slave1);
    EXPECT_EQ(bus->slave_count(), 2U);

    // Step 2: Attach StubRawSocket and enumerate via resoem::Enumerator.
    virtual_bus::StubRawSocket socket(bus, true);
    Enumerator enumerator(socket);
    Result<size_t> enumRes = enumerator.enumerate();
    ASSERT_TRUE(enumRes.has_value());
    EXPECT_EQ(enumRes.value(), 2U);

    // Step 3: Command all virtual slaves cleanly to SAFE_OP then OP.
    EXPECT_TRUE(enumerator.request_state_all(0x02U).has_value());
    EXPECT_TRUE(enumerator.request_state_all(0x04U).has_value());
    EXPECT_TRUE(enumerator.request_state_all(0x08U).has_value());
    quasar::test_support::AsyncTestLogger::instance().logOutput("slavesInOp", static_cast<long long>(bus->slave_count()));
}

TEST(ResoemTutorialTest, Tutorial3_MasterServiceConfigAndEniDocumentParsing) {
    quasar::test_support::TestScopeGuard scope("ResoemTutorialTest.Tutorial3");
    // Step 1: Instantiate reflexive EthercatMasterService and configure redundancy options.
    std::shared_ptr<EthercatMasterService> master =
        EthercatMasterService::create("Master0");
    EXPECT_EQ(master->getType(), "EthercatMasterService");
    master->setInterface("lo");
    master->setRedundancy(true, "lo");
    master->setRedundancy(false, "");

    // Step 2: Parse a minimal ENI document XML string via EniDocument::load_from_memory.
    const std::string minimalEni =
        "<EtherCATConfig><Config><Master><Info><Name>SimMaster</Name></Info></Master></Config></EtherCATConfig>";
    eni::EniDocument doc{};
    Result<> parsed = doc.load_from_memory(minimalEni);
    EXPECT_TRUE(parsed.has_value());
    EXPECT_TRUE(doc.save_to_string().has_value());
    quasar::test_support::AsyncTestLogger::instance().logOutput("eniParsed", parsed.has_value());
}

TEST(ResoemCoverageTest, FaultInjectionOutOfBoundsAndRedundancyToggles) {
    quasar::test_support::TestScopeGuard scope("ResoemCoverageTest.FaultsAndBounds");
    // Step 1: Exercise VirtualEtherCATBus cable break / heal / clear and OOB index.
    std::shared_ptr<virtual_bus::VirtualEtherCATBus> bus =
        virtual_bus::VirtualEtherCATBus::create();
    std::shared_ptr<virtual_bus::SimulatedSlave> s0 =
        virtual_bus::SimulatedSlave::create("SlaveA");
    bus->attach_slave(s0);
    EXPECT_EQ(bus->get_slave(0U), s0);
    EXPECT_EQ(bus->get_slave(99U), nullptr);

    bus->break_cable_after(0U);
    EXPECT_TRUE(bus->is_cable_broken());
    bus->restore_cable();
    EXPECT_FALSE(bus->is_cable_broken());

    // Step 2: Exercise StubRawSocket accessors and redundancy flags.
    virtual_bus::StubRawSocket sock(bus, false);
    EXPECT_FALSE(sock.has_redundancy());
    sock.set_redundancy(true);
    EXPECT_TRUE(sock.has_redundancy());
    sock.set_timeout(5);
    EXPECT_EQ(sock.interface_name(), "virtual_eth0");
    EXPECT_EQ(sock.get_bus(), bus);
    std::array<uint8_t, ETHERNET_MAC_ADDRESS_LENGTH> macPri = sock.get_mac_address(false);
    std::array<uint8_t, ETHERNET_MAC_ADDRESS_LENGTH> macSec = sock.get_mac_address(true);
    EXPECT_NE(macPri, macSec);

    bus->clear_slaves();
    EXPECT_EQ(bus->slave_count(), 0U);
}

TEST(ResoemFuzzStressTest, DeterministicFrameFuzzAndConcurrentVirtualBusStress) {
    quasar::test_support::TestScopeGuard scope("ResoemFuzzStressTest.FuzzAndConcurrent");
    // Step 1: Deterministic FrameBuilder payload fuzzing.
    quasar::test_support::DeterministicRng rng(RESOEM_FUZZ_SEED);
    for (std::size_t iter = 0U; iter < MAX_FRAME_FUZZ_ITERS; ++iter) {
        if (iter >= MAX_FRAME_FUZZ_ITERS) {
            throw std::runtime_error("Frame fuzz bound exceeded");
        } else {
            std::size_t len = static_cast<std::size_t>(rng.nextInt(4, 48));
            std::vector<byte> payload(len, static_cast<byte>(rng.nextInt(0, 255)));
            FrameBuilder fb{};
            fb.add_datagram(0x01U, static_cast<uint8_t>(iter & 0xFFU), 0U, 0x0100U, payload);
            std::span<const byte> raw = fb.build();
            EXPECT_GT(raw.size(), len);
        }
    }

    // Step 2: Concurrent process_frame invocations over shared VirtualEtherCATBus.
    std::shared_ptr<virtual_bus::VirtualEtherCATBus> bus =
        virtual_bus::VirtualEtherCATBus::create();
    bus->attach_slave(virtual_bus::SimulatedSlave::create("ConcurrentSlave"));

    FrameBuilder req{};
    std::array<byte, 2U> zeros = {byte{0}, byte{0}};
    req.add_datagram(0x01U, 0U, 0U, 0x0130U, zeros);
    std::vector<byte> txFrame(req.build().begin(), req.build().end());

    std::vector<std::thread> workers;
    workers.reserve(BUS_STRESS_WORKERS);
    for (std::size_t w = 0U; w < BUS_STRESS_WORKERS; ++w) {
        if (w >= BUS_STRESS_WORKERS) {
            throw std::runtime_error("Worker spawn bound exceeded");
        } else {
            workers.emplace_back([&bus, &txFrame]() {
                for (std::size_t f = 0U; f < FRAMES_PER_WORKER; ++f) {
                    if (f >= FRAMES_PER_WORKER) {
                        throw std::runtime_error("Frame loop bound exceeded");
                    } else {
                        std::vector<byte> rx;
                        bus->process_frame(txFrame, false, rx);
                        EXPECT_EQ(rx.size(), txFrame.size());
                    }
                }
            });
        }
    }
    for (std::size_t w = 0U; w < workers.size(); ++w) {
        if (w >= BUS_STRESS_WORKERS) {
            throw std::runtime_error("Worker join bound exceeded");
        } else {
            workers[w].join();
        }
    }
}

} // namespace resoem::test
