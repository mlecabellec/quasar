/**
 * @file TestRawEthernetService.cpp
 * @brief Google Test suite for the RawEthernetService and packet mapping features.
 */

#include <gtest/gtest.h>
#include "quasar/named/RawEthernetService.hpp"
#include "quasar/named/traversal/PredefinedRules.hpp"

using namespace quasar::named;
using namespace quasar::named::traversal;

/**
 * @class TestRawEthernetService
 * @brief Test fixture for verifying raw ethernet socket operations and mappings.
 */
class TestRawEthernetService : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @test VerifyCreationAndPropertyMapping
 * @brief Tests that RawEthernetService creates all mandatory child nodes and endpoints.
 * @feature [TSK-20260529-001.2] Dual Buffer Nodes.
 */
TEST_F(TestRawEthernetService, VerifyCreationAndPropertyMapping) {
    std::shared_ptr<RawEthernetService> service = RawEthernetService::create("ethService");
    
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->getName(), "ethService");
    EXPECT_EQ(service->getType(), "RawEthernetService");

    // Verify sub-properties nodes
    std::shared_ptr<NamedString> ifName = service->getInterfaceNameNode();
    ASSERT_NE(ifName, nullptr);
    EXPECT_EQ(ifName->getName(), "interfaceName");
    EXPECT_EQ(ifName->toString(), "lo");

    std::shared_ptr<NamedInteger<uint16_t>> ethType = service->getEtherTypeNode();
    ASSERT_NE(ethType, nullptr);
    EXPECT_EQ(ethType->getName(), "etherType");
    EXPECT_EQ(ethType->value(), 0x0003); // ETH_P_ALL

    std::shared_ptr<NamedBuffer> inFrame = service->getIncomingFrameNode();
    ASSERT_NE(inFrame, nullptr);
    EXPECT_EQ(inFrame->getName(), "incomingFrame");
    EXPECT_EQ(inFrame->size(), 1518);

    std::shared_ptr<NamedBuffer> outFrame = service->getOutgoingFrameNode();
    ASSERT_NE(outFrame, nullptr);
    EXPECT_EQ(outFrame->getName(), "outgoingFrame");
    EXPECT_EQ(outFrame->size(), 1518);

    std::shared_ptr<NamedObject> inTree = service->getIncomingTreeNode();
    ASSERT_NE(inTree, nullptr);
    EXPECT_EQ(inTree->getName(), "incomingTree");

    // Verify exposed send and stop/start hooks
    EXPECT_NE(service->getChild("send"), nullptr);
    EXPECT_NE(service->getChild("run"), nullptr);
    EXPECT_NE(service->getChild("start"), nullptr);
    EXPECT_NE(service->getChild("stop"), nullptr);
}

/**
 * @test VerifyPermissionSkippingOnStart
 * @brief Ensures socket initialization fails gracefully or skips if lacking permissions.
 * @feature [TSK-20260529-001.1] Raw AF_PACKET Socket.
 */
TEST_F(TestRawEthernetService, VerifyPermissionSkippingOnStart) {
    std::shared_ptr<RawEthernetService> service = RawEthernetService::create("ethService");
    ASSERT_NE(service, nullptr);

    // Try starting the service. If it fails due to lack of network permissions (EPERM),
    // we catch it gracefully and skip.
    try {
        service->start();
        EXPECT_TRUE(service->isRunning());
        service->stop();
        EXPECT_FALSE(service->isRunning());
    } catch (const std::runtime_error& e) {
        std::string err = e.what();
        if (err.find("Permission denied") != std::string::npos ||
            err.find("Operation not permitted") != std::string::npos) {
            std::cout << "[SKIPPED] Missing raw socket CAP_NET_RAW permissions: " << err << std::endl;
        } else {
            // Rethrow or fail if it is some other unexpected error
            FAIL() << "Unexpected exception during start: " << err;
        }
    }
}

/**
 * @test VerifyIncomingTreeMaterialization
 * @brief Tests the materialization of captured packet bytes using the TreeTransformer.
 * @feature [TSK-20260529-001.3] Incoming Packet Materialization.
 */
TEST_F(TestRawEthernetService, VerifyIncomingTreeMaterialization) {
    std::shared_ptr<RawEthernetService> service = RawEthernetService::create("ethService");
    ASSERT_NE(service, nullptr);

    // Define transformation rules for parsing raw ethernet packets.
    // Standard Ethernet II Header offset mapping:
    // Dst MAC (6 bytes), Src MAC (6 bytes), EtherType (2 bytes)
    std::vector<FieldMapping> mappings = {
        {"destMacUpper", "int32", 0, quasar::coretypes::Endianness::BigEndian},
        {"srcMacUpper", "int32", 6, quasar::coretypes::Endianness::BigEndian},
        {"etherType", "int32", 12, quasar::coretypes::Endianness::BigEndian}
    };

    // Add cast to structure rule
    service->addRule(PredefinedRules::castToStructure("incomingFrame", mappings));

    // Mock incoming frame payload injection
    // 00 11 22 33 44 55 (Dst MAC),  66 77 88 99 aa bb (Src MAC),  00 00 08 00 (32-bit Big Endian EtherType representation)
    std::vector<uint8_t> mockPacket = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
        0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
        0x00, 0x00, 0x08, 0x00
    };

    // Update payload directly
    service->getIncomingFrameNode()->setBufferData(mockPacket);

    // Trigger transformer directly to test mapping logic
    std::shared_ptr<NamedBuffer> inBufferNode = service->getIncomingFrameNode();
    std::shared_ptr<NamedObject> inTreeContainer = service->getIncomingTreeNode();

    Transformer dummyTransformer;
    std::vector<std::shared_ptr<NamedObject>> materialized = 
        PredefinedRules::castToStructure("incomingFrame", mappings).apply(
            TransformContext(inBufferNode, 0, "incomingFrame"), dummyTransformer);

    ASSERT_EQ(materialized.size(), 1);
    std::shared_ptr<NamedObject> castedBuffer = materialized[0];
    ASSERT_NE(castedBuffer, nullptr);

    // Verify pseudo-primitives are correctly bound
    std::shared_ptr<NamedObject> destMacNode = castedBuffer->getChild("destMacUpper");
    ASSERT_NE(destMacNode, nullptr);
    std::shared_ptr<NamedInteger<int32_t>> destMac = std::dynamic_pointer_cast<NamedInteger<int32_t>>(destMacNode);
    ASSERT_NE(destMac, nullptr);
    EXPECT_TRUE(destMac->isBound());
    EXPECT_EQ(destMac->value(), 0x00112233);

    std::shared_ptr<NamedObject> srcMacNode = castedBuffer->getChild("srcMacUpper");
    ASSERT_NE(srcMacNode, nullptr);
    std::shared_ptr<NamedInteger<int32_t>> srcMac = std::dynamic_pointer_cast<NamedInteger<int32_t>>(srcMacNode);
    ASSERT_NE(srcMac, nullptr);
    EXPECT_EQ(srcMac->value(), 0x66778899);

    std::shared_ptr<NamedObject> ethTypeNode = castedBuffer->getChild("etherType");
    ASSERT_NE(ethTypeNode, nullptr);
    std::shared_ptr<NamedInteger<int32_t>> ethType = std::dynamic_pointer_cast<NamedInteger<int32_t>>(ethTypeNode);
    ASSERT_NE(ethType, nullptr);
    EXPECT_EQ(ethType->value(), 0x0800);
}
