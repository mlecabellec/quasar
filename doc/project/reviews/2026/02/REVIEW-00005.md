# REVIEW-00005: Review of SMP Standard - Common Concepts and Type System. (WIP)

## Description

This review covers the SMP Standard common concepts and type system.

## Review scope

### Reviewed code

- [REVIEW-00005.1] Smp implementation at commit [a723425c3bbdd16325457402ae73075d1ecc2c4f].

### Reviewed features and constraints

- [REVIEW-00005.2] Review of the [FE-0050] feature, including:
  - [FE-0050.1] Primitive Types specification:
    - [FE-0050.1.1] Fields, parameters, constants and properties shall be Primitive or User Defined Types.
    - [FE-0050.1.2] Mapping between SMP, XML, and ISO/ANSI C++ types.
    - [FE-0050.1.3] Primitive Types (Char8, String8, Bool, Int8, UInt8, Int16, UInt16, Int32, UInt32, Int64, UInt64, Float32, Float64).
    - [FE-0050.1.4] Duration type (signed 64-bit integer, nanoseconds).
    - [FE-0050.1.5] DateTime type (relative to 01.01.2000 12:00, nanoseconds).
    - [FE-0050.1.6] SMP Simple Field shall be of simple type.
    - [FE-0050.1.7] AnySimple type.
    - [FE-0050.1.8] AnySimpleArray type.
  - [FE-0050.2] Time Kinds:
    - [FE-0050.2.1] Simulation time (start of simulation).
    - [FE-0050.2.2] Mission time (relative to Mission Start).
    - [FE-0050.2.3] Zulu time (host or external clock).
    - [FE-0050.2.4] Epoch time (Simulation time with offset).
    - [FE-0050.2.5] ITimeKeeper SetEpochTime method.
  - [FE-0050.3] Path string:
    - [FE-0050.3.1] Valid route from one SMP object to another.
    - [FE-0050.3.2] Absolute and Relative path support.
    - [FE-0050.3.3] "/" delimiter between component names.
    - [FE-0050.3.4] "/" or "." delimiter for non-component children.
    - [FE-0050.3.5] Trailing delimiters allowed.
    - [FE-0050.3.6] Parent reference ("..").
    - [FE-0050.3.7] Current object reference (".").
    - [FE-0050.3.8] Array element identification ("[n]").
  - [FE-0050.4] Universally Unique Identifiers (UUID):
    - [FE-0050.4.1] All SMP types shall have a unique UUID.
  - [FE-0050.5] Exception specification:
    - [FE-0050.5.1] All SMP exceptions shall inherit from the Exception class.
  - [FE-0050.6] Simulation Lifecycle:
    - [FE-0050.6.1] Simulation states (Building, Connecting, Initialising, Standby, Executing, Storing, Restoring, Reconnecting, Exiting, Aborting).

## Review results

- [REVIEW-00005.3] Initial assessment in progress.

### Issues found

### Issues resolved

### Issues not resolved

### Issues to be resolved in the future

## Conclusion

Work is in progress.
