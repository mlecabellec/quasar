#include <gtest/gtest.h>
#include "quasar/logic/SFC.hpp"
#include "quasar/logic/StateMachine.hpp"
#include "quasar/logic/EvaluationPool.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedMap.hpp"
#include "quasar/named/ICommand.hpp"

using namespace quasar::logic;
using namespace quasar::named;

TEST(LogicStressTest, LuaInfiniteLoopRecovery) {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    EvaluationPool::getInstance().setHardTimeout(std::chrono::milliseconds(100));

    std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx");
    std::shared_ptr<StateMachine> sm = StateMachine::create("SafetyWatchdog");
    sm->setContextRoot(ctxRoot);

    std::shared_ptr<State> nominal = State::create("NOMINAL");
    std::shared_ptr<State> fault = State::create("FAULT");

    nominal->setInvariant(Expression(lua, "(function() while true do end end)()"));
    nominal->setFailureState(fault);
    fault->setInvariant(Expression(lua, "true"));

    sm->setInitialState(nominal);
    sm->initialize();
    sm->start();

    sm->step(std::chrono::milliseconds(10));
    EXPECT_EQ(sm->getCurrentState(), fault);
}

TEST(LogicStressTest, InvalidTreeAccess) {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx");
    std::shared_ptr<StateMachine> sm = StateMachine::create("RobustnessTest");
    sm->setContextRoot(ctxRoot);

    std::shared_ptr<State> s1 = State::create("S1");
    std::shared_ptr<State> fault = State::create("FAULT");

    // Accessing nil directly might be tricky with Proxy, use a deliberate Lua error
    s1->setInvariant(Expression(lua, "1 + nil == 2"));
    s1->setFailureState(fault);
    fault->setInvariant(Expression(lua, "true"));

    sm->setInitialState(s1);
    sm->initialize();
    sm->start();

    sm->step(std::chrono::milliseconds(10));
    EXPECT_EQ(sm->getCurrentState(), fault);
}

class AtomicIncrement : public ICommand {
public:
    std::shared_ptr<NamedObject> execute(std::shared_ptr<NamedObject> context) override {
        std::shared_ptr<NamedObject> obj = context->getChild("counter");
        if (std::shared_ptr<NamedInteger<int>> intObj = std::dynamic_pointer_cast<NamedInteger<int>>(obj)) {
            int val = intObj->value();
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); 
            intObj->setValue(val + 1);
        }
        return nullptr;
    }
};

TEST(LogicStressTest, ParallelDeterminism) {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx");
    std::shared_ptr<NamedInteger<int>> counter = NamedInteger<int>::create("counter", 0, ctxRoot);

    std::shared_ptr<SFC> sfc = SFC::create("DeterministicSFC");
    sfc->setContextRoot(ctxRoot);

    std::shared_ptr<State> start = State::create("START");
    std::shared_ptr<State> endA = State::create("ENDA");
    std::shared_ptr<State> endB = State::create("ENDB");

    std::shared_ptr<Transition> tA = Transition::create("TA", endA);
    tA->setPreCondition(Expression(lua, "true"));
    tA->setAction(std::make_shared<AtomicIncrement>());

    std::shared_ptr<Transition> tB = Transition::create("TB", endB);
    tB->setPreCondition(Expression(lua, "true"));
    tB->setAction(std::make_shared<AtomicIncrement>());

    start->addTransition(tA);
    start->addTransition(tB);

    sfc->addInitialStep(start);
    sfc->initialize();
    sfc->start();

    sfc->step(std::chrono::milliseconds(10));
    EXPECT_EQ(counter->value(), 2);
}

TEST(LogicStressTest, TokenMerge) {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx");
    std::shared_ptr<SFC> sfc = SFC::create("MergeSFC");
    sfc->setContextRoot(ctxRoot);

    std::shared_ptr<State> bomb = State::create("BOMB");
    bomb->setInvariant(Expression(lua, "true"));

    // Self-splitting transition: 1 token tries to become 2 on each cycle
    std::shared_ptr<Transition> split1 = Transition::create("SPLIT1", bomb);
    split1->setPreCondition(Expression(lua, "true"));
    std::shared_ptr<Transition> split2 = Transition::create("SPLIT2", bomb);
    split2->setPreCondition(Expression(lua, "true"));

    bomb->addTransition(split1);
    bomb->addTransition(split2);

    sfc->addInitialStep(bomb);
    sfc->initialize();
    sfc->start();

    // Even after many cycles of splitting, std::set merges them
    for (int i = 0; i < 20; ++i) {
        sfc->step(std::chrono::milliseconds(1));
    }

    // Proves that token explosion is mathematically impossible by design
    const std::set<std::shared_ptr<State>>& active = sfc->getActiveStates();
    EXPECT_EQ(active.size(), 1);
    EXPECT_EQ((*active.begin())->getName(), "BOMB");
}
