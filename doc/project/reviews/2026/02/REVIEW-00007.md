# REVIEW-00007: Review of SMP Standard - Simulation Environment Services. (WIP)

## Description

This review covers the SMP Standard simulation environment services.

## Review scope

### Reviewed code

- [REVIEW-00007.1] Smp implementation at commit [a723425c3bbdd16325457402ae73075d1ecc2c4f].

### Reviewed features and constraints

- [REVIEW-00007.2] Review of the [FE-0070] feature, including:
  - [FE-0070.1] Logger (ILogger interface): Log message kind mapping, Log method, persistence.
  - [FE-0070.2] Time Keeper (ITimeKeeper): Epoch, Mission, Simulation, and Zulu time management.
  - [FE-0070.3] Scheduler (IScheduler): Event scheduling (Simulation, Mission, Epoch, Zulu, Immediate), cycle time, repeat count, persistence.
  - [FE-0070.4] Event Manager (IEventManager): Global event subscription, unsubscription, and emission (Emit).
  - [FE-0070.5] Resolver (IResolver): Absolute and relative path resolution.
  - [FE-0070.6] Link Registry (ILinkRegistry): Link management and tracking between components.
  - [FE-0070.7] Simulator (ISimulator): Main control interface, state transitions (Publish, Configure, Connect, Initialise, Run, etc.), component/service containers.
  - [FE-0070.8] Persistence Support: IStorageReader and IStorageWriter interfaces for state vector management.
  - [FE-0070.9] Publication (IPublication): Interface for publishing operations, properties, and fields.
  - [FE-0070.10] Type Registry (ITypeRegistry): Registry for SMP value types, support for complex types (Float, Integer, Enumeration, Array, String, Structure, Class).
  - [FE-0070.11] Component Factory (IFactory): Factory interface for component lifecycle management.
  - [FE-0070.12] Event Loop: Processing of global events and time-based events in different simulation states.
  - [FE-0070.13] Threading: Thread safety requirements for models and services.

## Review results

- [REVIEW-00007.3] Initial assessment in progress.

### Issues found

### Issues resolved

### Issues not resolved

### Issues to be resolved in the future

## Conclusion

Work is in progress.
