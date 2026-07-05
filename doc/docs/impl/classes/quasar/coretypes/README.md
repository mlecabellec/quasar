# Core Types Classes

This directory contains the implementation details of the `quasar::coretypes` module, which provides the fundamental data types.

## Numeric Types
- [Number](Number.md): Abstract base class for all numeric types.
- [Integer](Integer.md): Template class for integral types.
- [IntegerTypes](IntegerTypes.md): List of standard integer aliases (Int8, UInt32, etc.).
- [FloatingPoint](FloatingPoint.md): Template class for floating-point types.
- [FloatingPointTypes](FloatingPointTypes.md): List of standard floating-point aliases (Float32, Float64).

## Buffer and Data Types
- [Buffer](Buffer.md): Wrapper for byte arrays with numeric I/O.
- [BufferSlice](BufferSlice.md): Non-owning view into a byte buffer.
- [BitBuffer](BitBuffer.md): Specialized buffer for bit-level manipulation.
- [Boolean](Boolean.md): Object wrapper for primitive booleans.
- [String](String.md): Immutable wrapper for strings.

## Temporal and Physical Types
- [Timestamp](Timestamp.md): Absolute time representation (microseconds since epoch).
- [Date](Date.md): Calendar date representation (days since epoch).
- [Duration](Duration.md): Time span representation (microseconds).
- [Quantity](Quantity.md): Physical value with associated SI unit and dimensional safety.
