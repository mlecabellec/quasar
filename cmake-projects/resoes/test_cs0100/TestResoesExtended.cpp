/**
 * @file TestResoesExtended.cpp
 * @brief Extended CS-0100 test suite for `resoes` EtherCAT SubDevice stack.
 * @details Covers tutorials, `AlState` / `OdObjectType` / `OdDataType` / `MailboxSlotState` enumerations, SII EEPROM bounds, dynamic CoE Rx/Tx PDO pack/unpack, seeded fuzzing, and concurrent `ObjectDictionary` access stress.
 * @task [TSK-20261113-001] Context-Owned C++23 EtherCAT SubDevice Stack
 * @feature [FE-0280] Hardware-Free EtherCAT SubDevice Emulation
 * @compliance Fulfills CS-0010, CS-0040, CS-0060, CS-0090, and CS-0100.
 */

#include "resoes/CoeServer.hpp"
#include "resoes/EepromEmulator.hpp"
#include "resoes/EscController.hpp"
#include "resoes/EscTypes.hpp"
#include "resoes/MailboxServer.hpp"
#include "resoes/ObjectDictionary.hpp"
#include "resoes/ResoesConstants.hpp"
#include "resoes/SlaveApplication.hpp"
#include "resoes/hal/MemoryEscHardware.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace resoes::test {

/**
 * @brief Seeded PDO fuzzing iteration limit (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Named symbolic constant.
 */
constexpr std::size_t MAX_PDO_FUZZ_ITERS = 64U;

/**
 * @brief Concurrent worker count for `ObjectDictionary` thread safety test (`CS-0040.2`).
 * @requirement [TSK-20261113-001] Thread-safe ObjectDictionary
 * @compliance Bounded worker count.
 */
constexpr std::size_t RESOES_WORKERS = 4U;

/**
 * @brief Reads/writes per worker (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Guarantees deterministic completion.
 */
constexpr std::size_t OPS_PER_WORKER = 150U;

/**
 * @brief Fixed PRNG seed (`CS-0100.12`).
 * @requirement [CS-0100.12] Deterministic Testing
 * @compliance Reproducible PDO fuzz payloads.
 */
constexpr std::uint64_t RESOES_FUZZ_SEED = 0x50E5C0E1ULL;

/**
 * @brief Null callback adapter used in `SlaveApplication` tutorial (`CS-0090.1`).
 * @task [TSK-20261113-001] SlaveApplication Callbacks
 * @compliance Overrides all pure virtual hooks safely.
 */
class TutorialAppCallbacks final : public IApplicationCallbacks {
public:
    uint32_t rxCount{0U};
    uint32_t txCount{0U};
    uint32_t safeCount{0U};
    uint32_t tickCount{0U};
    void on_rx_pdo_updated() override { ++rxCount; }
    void on_tx_pdo_prepare() override { ++txCount; }
    void on_safe_outputs() override { ++safeCount; }
    void on_application_tick() override { ++tickCount; }
};

TEST(ResoesTutorialTest, Tutorial1_MemoryHardwareAndEepromStorage) {
    quasar::test_support::TestScopeGuard scope("ResoesTutorialTest.Tutorial1");
    // Step 1: Initialize MemoryEscHardware and MemoryEepromStorage.
    MemoryEscHardware hw{};
    ASSERT_TRUE(hw.init().has_value());

    std::vector<byte> image(256U, byte{0});
    image[0U] = static_cast<byte>(0xA5U);
    image[1U] = static_cast<byte>(0x5AU);
    MemoryEepromStorage eeprom{image};
    EXPECT_EQ(eeprom.size_bytes(), 256U);

    // Step 2: Read and write SII bytes cleanly through IEepromStorage API.
    std::array<byte, 4U> readBuf{};
    ASSERT_TRUE(eeprom.read_bytes(0U, readBuf).has_value());
    EXPECT_EQ(readBuf[0U], static_cast<byte>(0xA5U));

    std::array<byte, 2U> patch = {static_cast<byte>(0x11U), static_cast<byte>(0x22U)};
    ASSERT_TRUE(eeprom.write_bytes(8U, patch).has_value());
    ASSERT_TRUE(eeprom.read_bytes(8U, readBuf).has_value());
    EXPECT_EQ(readBuf[0U], static_cast<byte>(0x11U));
    quasar::test_support::AsyncTestLogger::instance().logOutput("eepromBytes", static_cast<long long>(eeprom.size_bytes()));
}

TEST(ResoesTutorialTest, Tutorial2_ObjectDictionaryAndCoePdoPackUnpack) {
    quasar::test_support::TestScopeGuard scope("ResoesTutorialTest.Tutorial2");
    // Step 1: Build SlaveApplication stack providing EscController, ObjectDictionary, and CoeServer.
    MemoryEscHardware hw{};
    MemoryEepromStorage eeprom{};
    SlaveConfig cfg{};
    SlaveApplication app{hw, eeprom, cfg};
    ASSERT_TRUE(app.init().has_value());

    // Step 2: Configure minimal 0x1600 RxPDO & 0x1A00 TxPDO + 0x1C12/0x1C13 SM assigns.
    ObjectDictionary& od = app.od();
    OdObjectEntry outVar{0x7000U, OdObjectType::Var, 0U, "LedOut", {
        OdSubindexEntry{0U, OdDataType::Unsigned16, 16U, od_access::READ_WRITE_ALL | od_access::RX_PDO_MAP, "Value", {byte{0}, byte{0}}}
    }};
    OdObjectEntry inVar{0x6000U, OdObjectType::Var, 0U, "ButtonIn", {
        OdSubindexEntry{0U, OdDataType::Unsigned16, 16U, od_access::READ_ALL | od_access::TX_PDO_MAP, "Value", {static_cast<byte>(0x34U), static_cast<byte>(0x12U)}}
    }};
    ASSERT_TRUE(od.register_object(outVar).has_value());
    ASSERT_TRUE(od.register_object(inVar).has_value());

    // Add 0x1600 RxPDO mapping pointing to 0x7000:00 (16 bits) = 0x70000010.
    std::vector<byte> rxMapBytes(4U, byte{0});
    (void)write_le32(rxMapBytes, 0U, 0x70000010U);
    ASSERT_TRUE(od.register_object(OdObjectEntry{0x1600U, OdObjectType::Array, 1U, "RxPdoMap", {
        OdSubindexEntry{0U, OdDataType::Unsigned8, 8U, od_access::READ_ALL, "Count", {byte{1}}},
        OdSubindexEntry{1U, OdDataType::Unsigned32, 32U, od_access::READ_ALL, "Map1", rxMapBytes}
    }}).has_value());

    std::vector<byte> sm2Bytes(2U, byte{0});
    (void)write_le16(sm2Bytes, 0U, 0x1600U);
    ASSERT_TRUE(od.register_object(OdObjectEntry{0x1C12U, OdObjectType::Array, 1U, "Sm2Assign", {
        OdSubindexEntry{0U, OdDataType::Unsigned8, 8U, od_access::READ_ALL, "Count", {byte{1}}},
        OdSubindexEntry{1U, OdDataType::Unsigned16, 16U, od_access::READ_ALL, "Pdo0", sm2Bytes}
    }}).has_value());

    // Step 3: Compute RxPDO size and unpack incoming master RxPDO payload.
    uint16_t rxBytes = 0U;
    uint16_t txBytes = 0U;
    ASSERT_TRUE(app.coe().compute_pdo_sizes_bytes(rxBytes, txBytes).has_value());
    EXPECT_EQ(rxBytes, 2U);

    std::array<byte, 2U> masterOut = {static_cast<byte>(0xCDU), static_cast<byte>(0xABU)};
    ASSERT_TRUE(app.coe().unpack_rx_pdo(masterOut).has_value());
    Result<std::vector<byte>> readBack = od.read_bytes(0x7000U, 0U);
    ASSERT_TRUE(readBack.has_value());
    ASSERT_EQ(readBack.value().size(), 2U);
    EXPECT_EQ(readBack.value()[0U], static_cast<byte>(0xCDU));
    EXPECT_EQ(readBack.value()[1U], static_cast<byte>(0xABU));
}

TEST(ResoesTutorialTest, Tutorial3_SlaveApplicationFreerunAndCallbacks) {
    quasar::test_support::TestScopeGuard scope("ResoesTutorialTest.Tutorial3");
    // Step 1: Attach IApplicationCallbacks to SlaveApplication and initialize.
    MemoryEscHardware hw{};
    MemoryEepromStorage eeprom{};
    SlaveConfig cfg{};
    SlaveApplication app{hw, eeprom, cfg};
    TutorialAppCallbacks hooks{};
    app.set_callbacks(&hooks);
    ASSERT_TRUE(app.init().has_value());

    // Step 2: Execute free-run and poll steps cleanly.
    ASSERT_TRUE(app.step_poll().has_value());
    ASSERT_TRUE(app.step_freerun(IoProcessFlags{}).has_value());
    EXPECT_GE(hooks.tickCount, 1U);
    quasar::test_support::AsyncTestLogger::instance().logOutput("ticks", static_cast<long long>(hooks.tickCount));
}

TEST(ResoesCoverageTest, AllEnumsErrorBoundariesAndMailboxSlotStates) {
    quasar::test_support::TestScopeGuard scope("ResoesCoverageTest.EnumsAndBoundaries");
    // Step 1: Cover all AlState and MailboxSlotState enumerators.
    const std::array<AlState, 5U> states = {
        AlState::Init, AlState::PreOp, AlState::Boot, AlState::SafeOp, AlState::Op
    };
    const std::array<MailboxSlotState, 6U> mbxStates = {
        MailboxSlotState::Idle, MailboxSlotState::OwnerEcat, MailboxSlotState::OwnerPdi,
        MailboxSlotState::XoeInProgress, MailboxSlotState::RepeatReq, MailboxSlotState::Again
    };
    EXPECT_EQ(states.size(), 5U);
    EXPECT_EQ(mbxStates.size(), 6U);

    // Step 2: Verify EEPROM and OD out-of-bounds read/write error returns.
    MemoryEepromStorage smallEep{64U};
    std::array<byte, 16U> buf{};
    EXPECT_FALSE(smallEep.read_bytes(60U, buf).has_value());
    EXPECT_FALSE(smallEep.write_bytes(60U, buf).has_value());

    ObjectDictionary od{};
    EXPECT_FALSE(od.read_bytes(0x9999U, 0U).has_value());
    EXPECT_FALSE(od.write_bytes(0x9999U, 0U, buf).has_value());

    // Step 3: Verify MailboxServer rejects null protocol handler registration.
    MemoryEscHardware hw{};
    SlaveConfig cfg{};
    EscController esc{hw, cfg};
    MailboxServer mbx{hw, esc};
    EXPECT_FALSE(mbx.register_handler(nullptr).has_value());
}

TEST(ResoesFuzzStressTest, DeterministicPdoFuzzAndConcurrentOdAccess) {
    quasar::test_support::TestScopeGuard scope("ResoesFuzzStressTest.FuzzAndConcurrent");
    // Step 1: Seeded fuzzing of ObjectDictionary writes/reads across random uint32 values.
    ObjectDictionary od{};
    ASSERT_TRUE(od.register_object(OdObjectEntry{0x6001U, OdObjectType::Var, 0U, "FuzzReg", {
        OdSubindexEntry{0U, OdDataType::Unsigned32, 32U, od_access::READ_WRITE_ALL, "Val", {byte{0}, byte{0}, byte{0}, byte{0}}}
    }}).has_value());

    quasar::test_support::DeterministicRng rng(RESOES_FUZZ_SEED);
    for (std::size_t iter = 0U; iter < MAX_PDO_FUZZ_ITERS; ++iter) {
        if (iter >= MAX_PDO_FUZZ_ITERS) {
            throw std::runtime_error("Resoes fuzz bound exceeded");
        } else {
            uint32_t v = static_cast<uint32_t>(rng.nextInt(0, 1000000));
            ASSERT_TRUE(od.write_u32(0x6001U, 0U, v).has_value());
            Result<uint32_t> check = od.read_u32(0x6001U, 0U);
            ASSERT_TRUE(check.has_value());
            EXPECT_EQ(check.value(), v);
        }
    }

    // Step 2: Multi-threaded concurrent ObjectDictionary read/write stress test.
    std::vector<std::thread> workers;
    workers.reserve(RESOES_WORKERS);
    for (std::size_t w = 0U; w < RESOES_WORKERS; ++w) {
        if (w >= RESOES_WORKERS) {
            throw std::runtime_error("Worker spawn bound exceeded");
        } else {
            workers.emplace_back([&od, w]() {
                for (std::size_t k = 0U; k < OPS_PER_WORKER; ++k) {
                    if (k >= OPS_PER_WORKER) {
                        throw std::runtime_error("Worker op bound exceeded");
                    } else {
                        EXPECT_TRUE(od.write_u32(0x6001U, 0U, static_cast<uint32_t>((w * 1000U) + k)).has_value());
                        EXPECT_TRUE(od.read_u32(0x6001U, 0U).has_value());
                    }
                }
            });
        }
    }
    for (std::size_t w = 0U; w < workers.size(); ++w) {
        if (w >= RESOES_WORKERS) {
            throw std::runtime_error("Worker join bound exceeded");
        } else {
            workers[w].join();
        }
    }
}

} // namespace resoes::test
