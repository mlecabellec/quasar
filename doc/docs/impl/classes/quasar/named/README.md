# Named Classes

This directory contains the implementation details of the `quasar::named` module, which provides the hierarchical object system.

## Core Identity
- [NamedObject](NamedObject.md): The fundamental base class for all named hierarchical objects.
- [ActiveEntity](ActiveEntity.md): Base class for objects with lifecycle and reflexivity.

## Behavioral Objects
- [NamedMethod](NamedMethod.md): Encapsulates executable logic as an object.
- [NamedService](NamedService.md): Autonomous background service using method hooks.

## Dynamic Containers
- [NamedVariant](NamedVariant.md): Dynamic type wrapper for a single object.
- [NamedArray](NamedArray.md): Indexed collection of objects.
- [NamedMap](NamedMap.md): Keyed dictionary of objects.

## Data Types
- [NamedInteger](NamedInteger.md): Hierarchical integer value.
- [NamedBoolean](NamedBoolean.md): Hierarchical boolean value.
- [NamedString](NamedString.md): Hierarchical string value.
- [NamedFloatingPoint](NamedFloatingPoint.md): Hierarchical floating-point value.
- [NamedBuffer](NamedBuffer.md): Hierarchical byte buffer.
- [NamedBitBuffer](NamedBitBuffer.md): Hierarchical bit buffer.

## Temporal and Physical Types
- [NamedTimestamp](NamedTimestamp.md): Hierarchical absolute point in time.
- [NamedDate](NamedDate.md): Hierarchical calendar date.
- [NamedDuration](NamedDuration.md): Hierarchical time span.
- [NamedQuantity](NamedQuantity.md): Hierarchical physical value with units.

## Views and Slices
- [NamedBufferSlice](NamedBufferSlice.md): Named view into a byte buffer.
- [NamedBitBufferSlice](NamedBitBufferSlice.md): Named view into a bit buffer.

## Infrastructure
- [Serialization](Serialization.md): Conversion to/from XML, YAML, and JSON.
- [Traversal](Traversal.md): Tree navigation and manipulation algorithms.
