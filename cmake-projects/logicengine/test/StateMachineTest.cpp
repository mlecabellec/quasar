#include <gtest/gtest.h>
#include "quasar/logic/StateMachine.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedMap.hpp"
#include "quasar/named/ICommand.hpp"

using namespace quasar::logic;
using namespace quasar::named;

// Simple command to increment a value in the tree
class IncrementCommand : public ICommand {
public:
    explicit IncrementCommand(const std::string& path) : m_path(path) {}
    std::shared_ptr<NamedObject> execute(std::shared_ptr<NamedObject> context) override {
        if (!context) return nullptr;
        std::shared_ptr<NamedObject> obj = context->getChild(m_path);
        if (std::shared_ptr<NamedInteger<int>> intObj = std::dynamic_pointer_cast<NamedInteger<int>>(obj)) {
            intObj->setValue(intObj->value() + 1);
        }
        return nullptr;
    }
private:
    std::string m_path;
};

TEST(StateMachineTest, TransactionalSafety) {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx");
    std::shared_ptr<NamedInteger<int>> temp = NamedInteger<int>::create("temperature", 20, ctxRoot);

    std::shared_ptr<StateMachine> sm = StateMachine::create("CoolingSystem");
    sm->setContextRoot(ctxRoot);

    std::shared_ptr<State> idleState = State::create("IDLE");
    std::shared_ptr<State> coolingState = State::create("COOLING");
    std::shared_ptr<State> alarmState = State::create("ALARM");

    // IDLE Invariant: temperature < 50
    idleState->setInvariant(Expression(lua, "ctx.temperature < 50"));
    idleState->setFailureState(alarmState);

    // Transition IDLE -> COOLING if temperature > 30
    std::shared_ptr<Transition> t1 = Transition::create("StartCooling", coolingState);
    t1->setPreCondition(Expression(lua, "ctx.temperature > 30"));
    // Post-condition: temperature must be <= 31 after command
    t1->setPostCondition(Expression(lua, "ctx.temperature <= 31"));
    t1->setAction(std::make_shared<IncrementCommand>("temperature")); // INC instead of DEC to trigger failure
    
    idleState->addTransition(t1);

    sm->setInitialState(idleState);
    sm->initialize();
    sm->start();

    // 1. Nominal IDLE
    sm->step(std::chrono::milliseconds(10));
    EXPECT_EQ(temp->value(), 20);
    EXPECT_EQ(sm->getCurrentState(), idleState);

    // 2. Trigger Transition, but force Post-condition failure (Command increments to 32)
    temp->setValue(31);
    sm->step(std::chrono::milliseconds(10));
    
    // Command ran (31 -> 32), but Post-condition (ctx.temp <= 31) failed.
    EXPECT_EQ(temp->value(), 32);
    // Should move to destination's failure state (coolingState has none, so GROUND)
    EXPECT_NE(sm->getCurrentState(), coolingState);
    EXPECT_NE(sm->getCurrentState(), idleState);
}

TEST(StateMachineTest, InvariantFailure) {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx");
    std::shared_ptr<NamedInteger<int>> temp = NamedInteger<int>::create("temperature", 20, ctxRoot);

    std::shared_ptr<StateMachine> sm = StateMachine::create("SafetySM");
    sm->setContextRoot(ctxRoot);

    std::shared_ptr<State> safeState = State::create("SAFE");
    std::shared_ptr<State> faultState = State::create("FAULT");

    safeState->setInvariant(Expression(lua, "ctx.temperature < 100"));
    safeState->setFailureState(faultState);
    faultState->setInvariant(Expression(lua, "true"));

    sm->setInitialState(safeState);
    sm->initialize();
    sm->start();

    EXPECT_EQ(sm->getCurrentState(), safeState);

    // Trigger invariant failure
    temp->setValue(150);
    sm->step(std::chrono::milliseconds(10));

    // Should have moved to FAULT
    EXPECT_EQ(sm->getCurrentState(), faultState);
}
