#include <gtest/gtest.h>
#include "quasar/logic/BitVector.hpp"
#include "quasar/logic/CauseEffectMatrix.hpp"
#include "quasar/logic/LogicEngine.hpp"

using namespace quasar::logic;

TEST(BitVectorTest, BasicOperations) {
    BitVector bv(10);
    EXPECT_EQ(bv.size(), 10);
    
    bv.set(1, true);
    bv.set(5, true);
    
    EXPECT_TRUE(bv.get(1));
    EXPECT_TRUE(bv.get(5));
    EXPECT_FALSE(bv.get(0));
    EXPECT_TRUE(bv.any());
    
    BitVector bv2(10);
    bv2.set(1, true);
    bv2.set(2, true);
    
    BitVector andResult = bv & bv2;
    EXPECT_TRUE(andResult.get(1));
    EXPECT_FALSE(andResult.get(5));
    EXPECT_FALSE(andResult.get(2));
    
    BitVector orResult = bv | bv2;
    EXPECT_TRUE(orResult.get(1));
    EXPECT_TRUE(orResult.get(2));
    EXPECT_TRUE(orResult.get(5));
}

TEST(CauseEffectMatrixTest, Evaluation) {
    std::shared_ptr<CauseEffectMatrix> matrix = CauseEffectMatrix::create("TestMatrix", 4, 2);
    matrix->initialize();
    
    // Effect 0: AND(Cause 0, Cause 1)
    BitVector andMask(4);
    andMask.set(0, true);
    andMask.set(1, true);
    matrix->setAndMask(0, andMask);
    
    // Effect 1: OR(Cause 2, Cause 3)
    BitVector orMask(4);
    orMask.set(2, true);
    orMask.set(3, true);
    matrix->setOrMask(1, orMask);
    
    matrix->start();
    
    // Case 1: All inputs false
    matrix->step(std::chrono::milliseconds(10));
    EXPECT_FALSE(matrix->getEffect(0));
    EXPECT_FALSE(matrix->getEffect(1));
    
    // Case 2: Only Cause 0 true
    matrix->setCause(0, true);
    matrix->step(std::chrono::milliseconds(10));
    EXPECT_FALSE(matrix->getEffect(0));
    
    // Case 3: Cause 0 and 1 true
    matrix->setCause(1, true);
    matrix->step(std::chrono::milliseconds(10));
    EXPECT_TRUE(matrix->getEffect(0));
    
    // Case 4: Only Cause 2 true
    matrix->setCause(2, true);
    matrix->step(std::chrono::milliseconds(10));
    EXPECT_TRUE(matrix->getEffect(1));
}

TEST(LogicEngineTest, Orchestration) {
    std::shared_ptr<LogicEngine> engine = LogicEngine::create("Engine");
    std::shared_ptr<CauseEffectMatrix> matrix = CauseEffectMatrix::create("Matrix", 2, 1);
    matrix->initialize();
    matrix->start();
    
    engine->addComponent(matrix);
    
    BitVector andMask(2);
    andMask.set(0, true);
    matrix->setAndMask(0, andMask);
    matrix->setCause(0, true);
    
    engine->runCycle(std::chrono::milliseconds(10));
    EXPECT_TRUE(matrix->getEffect(0));
}
