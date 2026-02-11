# REVIEW-00006: Review of SMP Standard - Component Model & Interfaces. (WIP)

## Description

This review covers the SMP Standard component model and interfaces.

## Review scope

### Reviewed code

- [REVIEW-00006.1] Smp implementation at commit [a723425c3bbdd16325457402ae73075d1ecc2c4f].

### Reviewed features and constraints

- [REVIEW-00006.2] Review of the [FE-0060] feature, including:
  - [FE-0060.1] Object Specification (IObject): name, description, parent, child collection features; unique names; validity checks.
  - [FE-0060.2] Collection Specification (ICollection): at, size, empty, begin, end methods.
  - [FE-0060.3] Component Specification: IComponent interface; state (Created, Publishing, etc.); Publish, Configure, Connect, Disconnect; child management; IModel and IService interfaces.
  - [FE-0060.4] Aggregation: IAggregate and IReference interfaces for component referencing and management.
  - [FE-0060.5] Composition: IComposite and IContainer interfaces for composition of child components.
  - [FE-0060.6] Events: IEventSink, IEventSource, IEventConsumer, IEventProvider interfaces; subscription and notification.
  - [FE-0060.7] Entry Points: IEntryPoint and IEntryPointPublisher interfaces for schedulable tasks.
  - [FE-0060.8] Dynamic Invocation: IDynamicInvocation, IRequest interfaces; operation and property registration/invocation.
  - [FE-0060.9] Persistence: IPersist interface; symmetric Restore and Store methods.
  - [FE-0060.10] Failures: IFailure and IFallibleModel interfaces; state management and aggregation.
  - [FE-0060.11] Fields: ISimpleField, IStructureField, IArrayField, IForcibleField, IOutputField interfaces; value management, forcing, and data pushing.
  - [FE-0060.12] Properties: IProperty interface; access kind (Read/Write, etc.) and view kind.
  - [FE-0060.13] Operations: IOperation and IParameter interfaces; dynamic invocation and parameter directions.

## Review results

- [REVIEW-00006.3] Initial assessment in progress.

### Issues found

### Issues resolved

### Issues not resolved

### Issues to be resolved in the future

## Conclusion

Work is in progress.
