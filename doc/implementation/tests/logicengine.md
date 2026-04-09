# Logic Engine Module Tests

This document describes the testing suite for the `logicengine` module, located in `cmake-projects/logicengine/test/`.

## 1. State Machine Tests (`StateMachineTest.cpp`)
- **Lifecycle Hooks**: Verifies that `onEntry`, `onExit`, and `onDo` actions are executed at the correct time during transitions.
- **Invariants**: Validates that the machine transitions to the `failureState` if a state invariant evaluates to false.
- **Hierarchical Resolution**: Verifies that transition guards are evaluated from the innermost state outwards.
- **Contextual Evaluation**: Tests Lua-based guard expressions that reference variables in the `contextRoot` tree.

## 2. SFC Tests (`SFCTest.cpp`)
- **Parallel Steps**: Verifies that multiple steps can be active simultaneously and that tokens move correctly across transitions.
- **Transactional Cycles**: Ensures that all enabled transitions fire in the same execution cycle (`step`).
- **Conflict Resolution**: Validates deterministic behavior when multiple conflicting transitions are enabled.

## 3. Cause-Effect Matrix Tests (`LogicEngineTest.cpp`)
- **Binary Logic**: Verifies that AND and OR masks correctly compute effect bits based on cause inputs.
- **Bit-Parallel Performance**: Validates that the `BitVector` utility performs wide logic operations correctly.

## 4. Stress and Stability (`LogicStressTest.cpp`)
- **High Node Count**: Execution of a state machine with 1,000 states and 5,000 transitions.
- **Concurrent Modification**: Verifies that adding/removing logic components from the `LogicEngine` while it is running is thread-safe.
- **Memory Safety**: ASan validation of long-duration execution loops (100,000 cycles).
