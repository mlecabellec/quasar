# Logic Engine Classes

This directory contains the implementation details of the logic and control engine.

## Core Framework
- [LogicEngine](LogicEngine.md): High-level orchestrator and the `LogicComponent` base class.

## Operational Models
- [StateMachine](StateMachine.md): Hierarchical State Machine (HSM) implementation, including `State` and `Transition` details.
- [SFC](SFC.md): Sequential Function Chart (Grafcet) implementation supporting multi-token parallel logic.
- [CauseEffectMatrix](CauseEffectMatrix.md): High-performance combinatorial logic using bit-parallel evaluation via `BitVector`.
