# Datacodec Module Tests

This document describes the testing suite for the `datacodec` module, located in `cmake-projects/datacodec/test/`.

## 1. Unit Tests (`TestDatacodec.cpp`)

The unit tests verify the core functionality of codecs and schema-based mapping.

### Test Cases

- **Codec Basic Functionality**
    - **Sequence**: Create an `IntegerCodec<uint8_t>`, and call `decode` on a `BitBufferSlice` containing `0xAB`.
    - **Post-conditions**: Decoded value must be equal to `0xAB`. `NamedInteger` type must be preserved.

- **Schema Definition**
    - **Sequence**: Construct a `ContainerDef` and add multiple `FieldDef` objects with specified bit offsets and codecs.
    - **Verification**: Ensure fields are correctly registered and accessible within the schema.

- **Mapping (BinaryMapper)**
    - **Verification**: Ensure the mapper correctly traverses the schema and populates a result container with decoded fields from the source buffer.

## 2. Integration and Edge Cases

Tests cover the following aspects:
- **Buffer Overflow**: Verified that `std::out_of_range` is thrown when decoding from a buffer slice that is too small for the specified codec.
- **Type Safety**: Verified that `encode` throws `std::invalid_argument` when passed a `NamedObject` of a type that doesn't match the codec (e.g., passing `NamedBoolean` to `IntegerCodec`).
- **Conditional Fields**: (Manual/Planned) Verification of field presence based on runtime context in `BinaryMapper`.
