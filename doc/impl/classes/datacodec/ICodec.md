# ICodec

`ICodec` is the abstract interface for all binary data codecs. It defines the contract for transforming between raw binary representations (accessed via `BitBufferSlice`) and high-level object representations (`NamedObject` instances).

## Core Responsibilities

- **Decoding**: Converting bits into structured named objects.
- **Encoding**: Converting named objects into bits.
- **Sizing**: Reporting the bit size required for a specific value or the fixed bit size of the type.

## Interface Overview

### Methods

- `decode(const BitBufferSlice &buffer)`: Reads bits and constructs a `NamedObject`.
- `encode(const std::shared_ptr<NamedObject> &value, BitBufferSlice &buffer)`: Writes value to buffer.
- `getBitSize()`: returns fixed size in bits, or 0 if dynamic.
- `getEncodedBitSize(const std::shared_ptr<NamedObject> &value)`: Returns bits required for a specific value.

## Implementation Classes

- `IntegerCodec<T>`: For integral types.
- `FloatCodec<T>`: For floating-point types (`float`, `double`).
- `StringCodec`: For fixed-length or variable-length strings.
- `TransformCodec`: Middleware for applying transformations during codec operations.
