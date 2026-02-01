# REVIEW-00001: Preliminay review of quasar::coretypes namespace. (WIP)

## Description

This review is a preliminary review of the quasar::coretypes namespace.

## Review scope

### Reviewed code

- [REVIEW-00001.1] quasar::coretypes namespace at commit [9a2b7dfe29c2215bbbcc176a0fb3fe954872da71].

### Reviewed features and constraints

- [REVIEW-00001.2] Review of the whole [FE-0010] feature, including:
  - [FE-0010.1] Provide, for each basic numeric type (int, long, float, double, signed and unsigned, with various sizes), a class which is assignable to its basic type. 
      - [FE-0010.1.1] Each class related to a basic numeric type shall provide a constructor which takes a value of the basic type as parameter.
      - [FE-0010.1.2] Each class related to a basic numeric type shall be derivated from a common "Number" base class.
 
## Review results

- [REVIEW-00001.3], regarding [FE-0010.1],
  - coretypes/include/quasar/coretypes/IntegerTypes.hpp defines eight aliases handling int8, int16, int32, int64, uint8, uint16, uint32, uint64 primitive types. Aliases are based on quasar::coretypes::Integer template class.
  - coretypes/test/TestInteger.cpp build objects using classic constructors but not using assigment. It's not compliant with [FE-0010.1].
- [REVIEW-00001.4], regarding [FE-0010.1.2], quasar::coretypes::Integer template class is derivated from quasar::coretypes::Number template class. It's compliant with [FE-0010.1.2].

### Issues found



### Issues resolved

- [REVIEW-00001.3] Added test in TestInteger.cpp.

### Issues not resolved

### Issues to be resolved in the future


