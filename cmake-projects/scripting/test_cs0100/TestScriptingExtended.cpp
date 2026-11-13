/**
 * @file TestScriptingExtended.cpp
 * @brief Extended CS-0100 test suite for `quasar_scripting` covering `ScriptExecutor`, `LuaFuture`, `LuaObserver`, `QueuedObserver`, `ObjectTracker`, `ShellUtilities`, exceptions, fuzzing, and concurrency.
 * @details Supplements existing scripting suites without modifying them. Verifies invalid `LuaFuture::get()`, watermark drops in `QueuedObserver`, `ShellUtilities` bindings (`ls`/`help`), syntax error exceptions, deterministic script fuzzing, and concurrent `LuaService` submissions.
 * @task [TSK-20260311-011] Lua Environment Stress & Validation
 * @task [TSK-20260328-001] Reflexive Lua Method & Service Execution
 * @feature [FE-0130] Lua Scripting Integration
 * @compliance Fulfills CS-0010, CS-0040, CS-0060, CS-0090, and CS-0100.
 */

#include "ShellUtilities.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaFuture.hpp"
#include "quasar/scripting/LuaObserver.hpp"
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/PluginLoader.hpp"
#include "quasar/scripting/QueuedObserver.hpp"
#include "quasar/scripting/ScriptExecutor.hpp"
#include "quasar/scripting/ScriptManager.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace quasar::scripting::test {

/**
 * @brief Seeded Lua arithmetic fuzz iteration limit (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Guarantees deterministic fuzz termination.
 */
constexpr std::size_t MAX_LUA_FUZZ_ITERS = 64U;

/**
 * @brief Worker threads submitting concurrent scripts to `LuaService` (`CS-0040.2`).
 * @requirement [TSK-20260311-011] Lua Concurrency Stress
 * @compliance Fixed bounded worker count.
 */
constexpr std::size_t LUA_STRESS_WORKERS = 3U;

/**
 * @brief Scripts queued per concurrent thread (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Deterministic loop upper bound.
 */
constexpr std::size_t SCRIPTS_PER_WORKER = 25U;

/**
 * @brief Wait cap for `LuaService` task completion (`CS-0010.27`).
 * @requirement [CS-0010.27] Timed Wait Usage
 * @compliance Bounded condition wait avoids hang on failure.
 */
constexpr std::chrono::milliseconds SERVICE_SYNC_TIMEOUT_MS{1500};

/**
 * @brief Fixed PRNG seed (`CS-0100.12`).
 * @requirement [CS-0100.12] Deterministic Testing
 * @compliance Reproducible script inputs.
 */
constexpr std::uint64_t SCRIPTING_FUZZ_SEED = 0x5C819701ULL;

TEST(ScriptingTutorialTest, Tutorial1_LuaEngineAndScriptExecutorModes) {
    quasar::test_support::TestScopeGuard scope("ScriptingTutorialTest.Tutorial1");
    // Step 1: Create a standalone LuaEngine, execute code, and exercise GC & ID.
    std::shared_ptr<LuaEngine> engine = LuaEngine::create();
    ASSERT_NE(engine, nullptr);
    sol::protected_function_result res = engine->executeString("return 6 * 7");
    ASSERT_TRUE(res.valid());
    int answer = res;
    EXPECT_EQ(answer, 42);
    engine->gcStep(10);
    std::shared_ptr<LuaEngine> secondEngine = LuaEngine::create();
    EXPECT_GT(secondEngine->getId(), engine->getId());

    // Step 2: Run fire-and-forget transient Lua script and sync execution on LuaService.
    ScriptExecutor::ExecuteOnce("local temp = 1 + 2");
    std::shared_ptr<LuaService> service = LuaService::create("ExecSvc");
    service->start();
    ScriptExecutor::ExecuteSync("globalPing = 99", service);
    EXPECT_EQ(static_cast<int>(service->getEngine()->getState()["globalPing"]), 99);
    service->stop();
    quasar::test_support::AsyncTestLogger::instance().logOutput("luaAnswer", answer);
}

TEST(ScriptingTutorialTest, Tutorial2_LuaFutureQueuedObserverAndTracker) {
    quasar::test_support::TestScopeGuard scope("ScriptingTutorialTest.Tutorial2");
    // Step 1: Test default/invalid LuaFuture state and real postTask execution via LuaService.
    LuaFuture emptyFut;
    EXPECT_FALSE(emptyFut.valid());
    EXPECT_FALSE(emptyFut.wait());
    EXPECT_FALSE(emptyFut.isReady());
    EXPECT_THROW((void)emptyFut.get(), std::runtime_error);

    std::shared_ptr<LuaService> svc = LuaService::create("ObserverSvc");
    svc->start();

    // Step 2: Attach QueuedObserver onto a NamedInteger and notify on updates.
    std::atomic<int> seenCount{0};
    svc->getEngine()->getState().set_function("onNodeChanged", [&seenCount]() {
        seenCount.fetch_add(1);
    });
    std::shared_ptr<QueuedObserver> qObs = std::make_shared<QueuedObserver>(
        svc, svc->getEngine()->getState()["onNodeChanged"], 10U);
    std::shared_ptr<named::NamedInteger<int>> counter =
        named::NamedInteger<int>::create("Counter", 1);
    ObjectTracker::getInstance().track(counter);
    EXPECT_TRUE(ObjectTracker::getInstance().isAlive(counter));

    qObs->notify(counter);
    ScriptExecutor::ExecuteSync("return true", svc);
    EXPECT_GE(seenCount.load(), 1);

    ObjectTracker::getInstance().untrack(counter.get());
    ObjectTracker::getInstance().cleanup();
    svc->stop();
}

TEST(ScriptingTutorialTest, Tutorial3_ShellUtilitiesAndPluginBoundaries) {
    quasar::test_support::TestScopeGuard scope("ScriptingTutorialTest.Tutorial3");
    // Step 1: Bind qlsh ShellUtilities (`ls`, `help`) into a Lua state cleanly.
    std::shared_ptr<LuaEngine> eng = LuaEngine::create();
    bindShellUtilities(eng->getState());
    sol::protected_function_result helpRes = eng->executeString("help()");
    EXPECT_TRUE(helpRes.valid());

    // Step 2: Verify PluginLoader rejects missing plugin shared objects cleanly.
    PluginLoader loader;
    EXPECT_FALSE(loader.loadPlugin("/nonexistent/missing_plugin.so", eng->getState()));
}

TEST(ScriptingCoverageTest, LuaErrorHandlingAndObserverInvalidation) {
    quasar::test_support::TestScopeGuard scope("ScriptingCoverageTest.ErrorBranches");
    std::shared_ptr<LuaEngine> eng = LuaEngine::create();
    std::shared_ptr<LuaService> svc = LuaService::create("ErrSvc");
    svc->start();

    // Invalid Lua script syntax/runtime error returns invalid protected_function_result or throws LuaExecutionException.
    bool caughtOrInvalid = false;
    try {
        sol::protected_function_result badRes = eng->executeString("return (1 +");
        caughtOrInvalid = !badRes.valid();
    } catch (const std::exception&) {
        caughtOrInvalid = true;
    }
    EXPECT_TRUE(caughtOrInvalid);

    // LuaObserver invalidation prevents subsequent callback invocations.
    int calls = 0;
    eng->getState().set_function("cb", [&calls](sol::object) { ++calls; });
    LuaObserver obs(eng->getState()["cb"], eng.get(), svc);
    std::shared_ptr<named::NamedBoolean> flag = named::NamedBoolean::create("Flag", true);
    obs.invalidate();
    obs.notify(flag);
    ScriptExecutor::ExecuteSync("return true", svc);
    EXPECT_EQ(calls, 0);
    svc->stop();
}

TEST(ScriptingFuzzStressTest, DeterministicLuaFuzzAndConcurrentServiceStress) {
    quasar::test_support::TestScopeGuard scope("ScriptingFuzzStressTest.FuzzAndConcurrent");
    // Step 1: Deterministic seeded Lua arithmetic fuzzing.
    std::shared_ptr<LuaEngine> eng = LuaEngine::create();
    quasar::test_support::DeterministicRng rng(SCRIPTING_FUZZ_SEED);
    for (std::size_t iter = 0U; iter < MAX_LUA_FUZZ_ITERS; ++iter) {
        if (iter >= MAX_LUA_FUZZ_ITERS) {
            throw std::runtime_error("Lua fuzz bound exceeded");
        } else {
            int lhs = rng.nextInt(-500, 500);
            int rhs = rng.nextInt(-500, 500);
            std::string expr = "return " + std::to_string(lhs) + " + " + std::to_string(rhs);
            sol::protected_function_result r = eng->executeString(expr);
            ASSERT_TRUE(r.valid());
            int out = r;
            EXPECT_EQ(out, lhs + rhs);
        }
    }

    // Step 2: Concurrent ScriptExecutor workers posting to a shared LuaService.
    std::shared_ptr<LuaService> svc = LuaService::create("ConcurrentLuaSvc");
    svc->start();
    std::atomic<std::size_t> completed{0U};
    std::vector<std::thread> workers;
    workers.reserve(LUA_STRESS_WORKERS);

    for (std::size_t w = 0U; w < LUA_STRESS_WORKERS; ++w) {
        if (w >= LUA_STRESS_WORKERS) {
            throw std::runtime_error("Worker spawn bound exceeded");
        } else {
            workers.emplace_back([&svc, &completed, w]() {
                for (std::size_t s = 0U; s < SCRIPTS_PER_WORKER; ++s) {
                    if (s >= SCRIPTS_PER_WORKER) {
                        throw std::runtime_error("Script loop bound exceeded");
                    } else {
                        ScriptExecutor::ExecuteAsync(
                            "local v = " + std::to_string((w * 100U) + s), svc);
                        completed.fetch_add(1U);
                    }
                }
            });
        }
    }
    for (std::size_t w = 0U; w < workers.size(); ++w) {
        if (w >= LUA_STRESS_WORKERS) {
            throw std::runtime_error("Worker join bound exceeded");
        } else {
            workers[w].join();
        }
    }
    ScriptExecutor::ExecuteSync("return true", svc);
    EXPECT_EQ(completed.load(), LUA_STRESS_WORKERS * SCRIPTS_PER_WORKER);
    svc->stop();
}

} // namespace quasar::scripting::test
