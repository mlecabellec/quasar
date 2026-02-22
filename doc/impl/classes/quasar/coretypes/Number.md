# Number

`Number` is the abstract base class for all numeric types in the Quasar framework. It defines a unified polymorphic interface for numeric operations, conversions, and introspection.

## Design Goals

- **Abstraction**: Handle different numeric types (integers, floating-point) uniformly.
- **Safety**: Provide checked arithmetic operations.
- **Immutability**: Derived classes should be immutable to ensure thread-safety.

## Interface Overview

### Conversions

- `toInt()`, `toLong()`
- `toFloat()`, `toDouble()`
- `toString()`

### Arithmetic (Polymorphic)

Standard operations (returning `std::shared_ptr<Number>`):
- `add`, `subtract`, `multiply`, `divide`

Safe operations (throwing exceptions on errors):
- `safeAdd`, `safeSubtract`, `safeMultiply`, `safeDivide`

### Comparison

- `compareTo(const Number &other)`: Returns -1, 0, or 1.
- `equals(const Number &other)`: Returns `true` if values match.

### Bitwise Operations

- `bitwiseAnd`, `bitwiseOr`, `bitwiseXor`, `bitwiseNot`
- `bitwiseLeftShift`, `bitwiseRightShift`
*Note: These throw `std::runtime_error` if the underlying type does not support them (e.g., FloatingPoint).*

### Introspection

- `getType()`: Returns string like "Integer" or "FloatingPoint".
- `isIntegerType()`: Returns `true` if integral.
- `isSigned()`: Returns `true` if type carries a sign.

## Implementation Classes

- `Integer<T>`: Concrete implementation for integral types.
- `FloatingPoint<T>`: Concrete implementation for floating-point types.
