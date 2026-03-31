#include <gtest/gtest.h>
#include "quasar/logic/SFC.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedMap.hpp"

using namespace quasar::logic;
using namespace quasar::named;

TEST(SFCTest, ParallelExecution) {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx");
    std::shared_ptr<NamedInteger<int>> trigger = NamedInteger<int>::create("trigger", 0, ctxRoot);

    std::shared_ptr<SFC> sfc = SFC::create("ParallelSFC");
    sfc->setContextRoot(ctxRoot);

    std::shared_ptr<State> step1 = State::create("STEP1");
    std::shared_ptr<State> step2a = State::create("STEP2A");
    std::shared_ptr<State> step2b = State::create("STEP2B");

    step1->setInvariant(Expression(lua, "true"));
    step2a->setInvariant(Expression(lua, "true"));
    step2b->setInvariant(Expression(lua, "true"));

    // Two transitions from STEP1 that can fire simultaneously
    std::shared_ptr<Transition> tA = Transition::create("T_A", step2a);
    tA->setPreCondition(Expression(lua, "ctx.trigger == 1"));
    tA->setPostCondition(Expression(lua, "true"));

    std::shared_ptr<Transition> tB = Transition::create("T_B", step2b);
    tB->setPreCondition(Expression(lua, "ctx.trigger == 1"));
    tB->setPostCondition(Expression(lua, "true"));

    step1->addTransition(tA);
    step1->addTransition(tB);

    sfc->addInitialStep(step1);
    sfc->initialize();
    sfc->start();

    // Trigger split
    trigger->setValue(1);
    sfc->step(std::chrono::milliseconds(10));

    // Next cycle should have two active steps
    const std::set<std::shared_ptr<State>>& active = sfc->getActiveStates();
    EXPECT_EQ(active.size(), 2);
    // Use std::set::find or count since set doesn't have contains in C++17 (but we are in C++23)
    EXPECT_TRUE(active.contains(step2a));
    EXPECT_TRUE(active.contains(step2b));
}
