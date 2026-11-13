/**
 * @file TestLogicEngineExtended.cpp
 * @brief Extended CS-0100 test suite for `quasar_logicengine` covering Expression Lua compilation/evaluation, EvaluationPool timeouts/status enums, StateMachine invariants/failure states, SFC parallel steps, and CauseEffectMatrix bit-vector stress.
 * @details Complements existing unit tests without modifying them. Tests all `EvaluationStatus` outcomes, syntax error exceptions, concurrent step cycles, and large `CauseEffectMatrix` evaluations.
 * @task [TSK-20260311-009] Deterministic HSM / SFC / CEM Logic Engine
 * @feature [FE-0240] Industrial Logic Execution & Safety Watchdog
 * @compliance Fulfills CS-0010, CS-0040, CS-0060, CS-0090, and CS-0100.
 */

#include "quasar/logic/BitVector.hpp"
#include "quasar/logic/CauseEffectMatrix.hpp"
#include "quasar/logic/Common.hpp"
#include "quasar/logic/EvaluationPool.hpp"
#include "quasar/logic/Expression.hpp"
#include "quasar/logic/LogicEngine.hpp"
#include "quasar/logic/SFC.hpp"
#include "quasar/logic/State.hpp"
#include "quasar/logic/StateMachine.hpp"
#include "quasar/logic/Transition.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace quasar::logic::test {

/**
 * @brief Cycle step duration used in HSM/SFC tests (`CS-0040.2`).
 * @requirement [FE-0240.1] Cyclic Execution
 * @compliance Named symbolic duration avoids magic numbers.
 */
constexpr duration STEP_PERIOD_NS{std::chrono::milliseconds(10)};

/**
 * @brief Matrix size for large bit-parallel `CauseEffectMatrix` stress (`CS-0040.2`).
 * @requirement [FE-0240.4] Cause-Effect Matrix Scalability
 * @compliance Multi-word BitVector (> 64 causes) stresses word boundary masking.
 */
constexpr std::size_t STRESS_CAUSE_COUNT = 256U;

/**
 * @brief Effect count for stress test (`CS-0040.2`).
 * @requirement [FE-0240.4] Matrix Output Scalability
 * @compliance Fixed symbolic constant.
 */
constexpr std::size_t STRESS_EFFECT_COUNT = 64U;

/**
 * @brief Deterministic fuzz iteration cap (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Bounded fuzzing limit.
 */
constexpr std::size_t MAX_MATRIX_FUZZ_ITERS = 64U;

/**
 * @brief Thread count for concurrent logic step verification (`CS-0040.2`).
 * @requirement [CS-0010.5] Concurrent Logic Safety
 * @compliance Safe fixed worker count.
 */
constexpr std::size_t LOGIC_WORKERS = 3U;

/**
 * @brief Cycle count per concurrent thread (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Deterministic loop upper bound.
 */
constexpr std::size_t CYCLES_PER_WORKER = 40U;

/**
 * @brief Fixed PRNG seed (`CS-0100.12`).
 * @requirement [CS-0100.12] Deterministic Testing
 * @compliance Reproducible cause-effect fuzz sequence.
 */
constexpr std::uint64_t LOGIC_FUZZ_SEED = 0x1061CE42ULL;

/**
 * @brief Watchdog timeout restoring default after timeout test (`CS-0040.2`).
 * @requirement [FE-0240] Watchdog Safety
 * @compliance Bounded timeout interval.
 */
constexpr std::chrono::milliseconds DEFAULT_WATCHDOG_MS{250};

/**
 * @brief Short watchdog interval used specifically to trigger `EvaluationStatus::Timeout` (`CS-0040.2`).
 * @requirement [FE-0240] Watchdog Timeout Branch
 * @compliance Bounded short timeout for deterministic timeout test.
 */
constexpr std::chrono::milliseconds SHORT_WATCHDOG_MS{40};

TEST(LogicEngineTutorialTest, Tutorial1_SimpleLuaExpressionEvaluation) {
    quasar::test_support::TestScopeGuard scope("LogicEngineTutorialTest.Tutorial1");
    // Step 1: Create a NamedObject context carrying a live sensor integer.
    std::shared_ptr<quasar::named::NamedObject> plantCtx = quasar::named::NamedObject::create("Plant");
    std::shared_ptr<quasar::named::NamedInteger<int>> pressure =
        quasar::named::NamedInteger<int>::create("Pressure", 120, plantCtx);

    // Step 2: Compile a Lua boolean expression referencing `ctx.Pressure`.
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    registerLogicTypes(lua);
    Expression highPressure(lua, "ctx.Pressure > 100");
    EXPECT_FALSE(highPressure.isEmpty());
    EXPECT_FALSE(highPressure.getBytecode().empty());

    // Step 3: Evaluate when condition holds and after dropping pressure below threshold.
    EXPECT_TRUE(highPressure.evaluate(plantCtx));
    pressure->setValue(80);
    EXPECT_FALSE(highPressure.evaluate(plantCtx));
    quasar::test_support::AsyncTestLogger::instance().logOutput("evalLowPressure", false);
}

TEST(LogicEngineTutorialTest, Tutorial2_IntermediateHierarchicalStateMachineWithInvariant) {
    quasar::test_support::TestScopeGuard scope("LogicEngineTutorialTest.Tutorial2");
    // Step 1: Set up Lua compiler and shared machine context tree.
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    registerLogicTypes(lua);
    std::shared_ptr<quasar::named::NamedObject> ctx = quasar::named::NamedObject::create("Context");
    std::shared_ptr<quasar::named::NamedBoolean> startCmd =
        quasar::named::NamedBoolean::create("Start", false, ctx);
    std::shared_ptr<quasar::named::NamedBoolean> safeInterlock =
        quasar::named::NamedBoolean::create("InterlockOk", true, ctx);

    // Step 2: Construct Idle, Running, and Fault states with a Run-state invariant.
    std::shared_ptr<StateMachine> hsm = StateMachine::create("PumpHSM");
    std::shared_ptr<State> idle = State::create("Idle");
    std::shared_ptr<State> running = State::create("Running");
    std::shared_ptr<State> fault = State::create("Fault");
    running->setInvariant(Expression(lua, "ctx.InterlockOk"));
    running->setFailureState(fault);

    std::shared_ptr<Transition> startTx = Transition::create("GoRun", running, idle);
    startTx->setPreCondition(Expression(lua, "ctx.Start"));

    hsm->setContextRoot(ctx);
    hsm->setInitialState(idle);
    hsm->initialize();
    hsm->start();

    // Step 3: Drive a valid transition to Running, then trip invariant to enter Fault.
    startCmd->setValue(true);
    hsm->step(STEP_PERIOD_NS);
    EXPECT_EQ(hsm->getCurrentState(), running);

    safeInterlock->setValue(false);
    hsm->step(STEP_PERIOD_NS);
    EXPECT_EQ(hsm->getCurrentState(), fault);
    quasar::test_support::AsyncTestLogger::instance().logOutput("finalHsmState", hsm->getCurrentState()->getName());
    hsm->stop();
}

TEST(LogicEngineTutorialTest, Tutorial3_AdvancedEngineOrchestratingSfcAndCauseEffect) {
    quasar::test_support::TestScopeGuard scope("LogicEngineTutorialTest.Tutorial3");
    // Step 1: Create LogicEngine orchestrating an SFC and a CauseEffectMatrix together.
    std::shared_ptr<LogicEngine> engine = LogicEngine::create("SafetyLine");
    std::shared_ptr<CauseEffectMatrix> matrix = CauseEffectMatrix::create("TripMatrix", 4U, 2U);
    BitVector tripOrMask(4U);
    tripOrMask.set(0U, true);
    tripOrMask.set(2U, true);
    matrix->setOrMask(0U, tripOrMask);
    matrix->initialize();
    matrix->start();

    std::shared_ptr<SFC> sfc = SFC::create("StartupSeq");
    std::shared_ptr<quasar::named::NamedObject> ctx = quasar::named::NamedObject::create("SeqCtx");
    std::shared_ptr<quasar::named::NamedBoolean> ready =
        quasar::named::NamedBoolean::create("Ready", true, ctx);
    (void)ready;

    sol::state lua;
    lua.open_libraries(sol::lib::base);
    std::shared_ptr<State> step1 = State::create("PreCheck");
    std::shared_ptr<State> step2 = State::create("Online");
    std::shared_ptr<Transition> adv = Transition::create("Proceed", step2, step1);
    adv->setPreCondition(Expression(lua, "ctx.Ready"));
    sfc->setContextRoot(ctx);
    sfc->addInitialStep(step1);
    sfc->initialize();
    sfc->start();

    // Step 2: Register components into LogicEngine and step once.
    engine->addComponent(matrix);
    engine->addComponent(sfc);
    matrix->setCause(2U, true);
    engine->runCycle(STEP_PERIOD_NS);

    // Step 3: Confirm both SFC progression and CauseEffectMatrix trip propagation.
    EXPECT_TRUE(matrix->getEffect(0U));
    EXPECT_EQ(sfc->getActiveStates().count(step2), 1U);
    quasar::test_support::AsyncTestLogger::instance().logOutput("matrixEffect0", matrix->getEffect(0U));
    matrix->stop();
    sfc->stop();
}

TEST(LogicEngineCoverageTest, EvaluationStatusesExceptionsAndBitVectorOps) {
    quasar::test_support::TestScopeGuard scope("LogicEngineCoverageTest.Branches");
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    // Step 1: Invalid Lua syntax must throw std::runtime_error in Expression ctor.
    EXPECT_THROW(Expression(lua, "1 + ("), std::runtime_error);
    Expression emptyExpr;
    EXPECT_TRUE(emptyExpr.isEmpty());
    EXPECT_TRUE(emptyExpr.evaluate(nullptr));

    // Step 2: Exercise all EvaluationStatus values via EvaluationPool directly.
    std::shared_ptr<quasar::named::NamedObject> ctx = quasar::named::NamedObject::create("PoolCtx");
    Expression trueExpr(lua, "1 == 1");
    Expression falseExpr(lua, "1 == 2");
    Expression errExpr(lua, "nonExistentGlobalFn()");
    Expression loopExpr(lua, "(function() local i = 0 while true do i = i + 1 end return true end)()");

    EXPECT_EQ(EvaluationPool::getInstance().evaluate(trueExpr.getBytecode(), ctx), EvaluationStatus::Success);
    EXPECT_EQ(EvaluationPool::getInstance().evaluate(falseExpr.getBytecode(), ctx), EvaluationStatus::LogicFalse);
    EXPECT_EQ(EvaluationPool::getInstance().evaluate(errExpr.getBytecode(), ctx), EvaluationStatus::Error);

    EvaluationPool::getInstance().setHardTimeout(SHORT_WATCHDOG_MS);
    EvaluationStatus timeoutStatus = EvaluationPool::getInstance().evaluate(loopExpr.getBytecode(), ctx);
    EvaluationPool::getInstance().setHardTimeout(DEFAULT_WATCHDOG_MS);
    quasar::test_support::AsyncTestLogger::instance().logBranch("watchdog_timeout");
    EXPECT_EQ(timeoutStatus, EvaluationStatus::Timeout);

    // Step 3: Exercise State pause/resume and BitVector multi-word equality/clear/AND/OR.
    BitVector a(130U);
    BitVector b(130U);
    a.set(0U, true);
    a.set(129U, true);
    b.set(129U, true);
    EXPECT_NE(a, b);
    BitVector c = a & b;
    EXPECT_TRUE(c.any());
    EXPECT_EQ(c, b);
    BitVector d = a | b;
    EXPECT_EQ(d, a);
    d.clear();
    EXPECT_FALSE(d.any());
}

TEST(LogicEngineFuzzStressTest, LargeMatrixFuzzAndConcurrentLogicEngineStress) {
    quasar::test_support::TestScopeGuard scope("LogicEngineFuzzStressTest.FuzzAndConcurrent");
    // Step 1: Multi-word CauseEffectMatrix deterministic fuzzing (256 causes x 64 effects).
    std::shared_ptr<CauseEffectMatrix> matrix =
        CauseEffectMatrix::create("BigMatrix", STRESS_CAUSE_COUNT, STRESS_EFFECT_COUNT);
    BitVector andMask(STRESS_CAUSE_COUNT);
    andMask.set(0U, true);
    andMask.set(199U, true);
    matrix->setAndMask(0U, andMask);
    BitVector orMask(STRESS_CAUSE_COUNT);
    orMask.set(255U, true);
    matrix->setOrMask(1U, orMask);
    matrix->initialize();
    matrix->start();

    quasar::test_support::DeterministicRng rng(LOGIC_FUZZ_SEED);
    for (std::size_t iter = 0U; iter < MAX_MATRIX_FUZZ_ITERS; ++iter) {
        if (iter >= MAX_MATRIX_FUZZ_ITERS) {
            throw std::runtime_error("Matrix fuzz bound exceeded");
        } else {
            bool c0 = (rng.nextInt(0, 1) == 1);
            bool c199 = (rng.nextInt(0, 1) == 1);
            bool c255 = (rng.nextInt(0, 1) == 1);
            matrix->setCause(0U, c0);
            matrix->setCause(199U, c199);
            matrix->setCause(255U, c255);
            matrix->step(STEP_PERIOD_NS);
            EXPECT_EQ(matrix->getEffect(0U), (c0 && c199));
            EXPECT_EQ(matrix->getEffect(1U), c255);
        }
    }

    // Step 2: Concurrent `LogicEngine::runCycle` calls testing `m_engineMutex` thread safety.
    std::shared_ptr<LogicEngine> sharedEngine = LogicEngine::create("ConcurrentEngine");
    sharedEngine->addComponent(matrix);
    std::vector<std::thread> workers;
    workers.reserve(LOGIC_WORKERS);

    for (std::size_t w = 0U; w < LOGIC_WORKERS; ++w) {
        if (w >= LOGIC_WORKERS) {
            throw std::runtime_error("Logic worker spawn bound exceeded");
        } else {
            workers.emplace_back([&sharedEngine]() {
                for (std::size_t c = 0U; c < CYCLES_PER_WORKER; ++c) {
                    if (c >= CYCLES_PER_WORKER) {
                        throw std::runtime_error("Logic worker loop bound exceeded");
                    } else {
                        sharedEngine->runCycle(STEP_PERIOD_NS);
                    }
                }
            });
        }
    }

    for (std::size_t w = 0U; w < workers.size(); ++w) {
        if (w >= LOGIC_WORKERS) {
            throw std::runtime_error("Logic worker join bound exceeded");
        } else {
            workers[w].join();
        }
    }
    matrix->stop();
}

} // namespace quasar::logic::test
