# Calibration Module Tests

This document describes the testing suite for the `calibration` module, located in `cmake-projects/calibration/test/`.

## 1. Unit Tests (`TestCalibrations.cpp`)

### Mathematical Accuracy
- **Linear**: Verifies $y = ax + b$ for various scales and offsets, including negative values.
- **Polynomial**: Validates high-degree polynomials against known value tables.
- **Table (Interpolation)**: Verifies linear interpolation between points and clamping behavior at range boundaries.

### Bidirectional Symmetry
- **Inversion**: Verifies that `engToRaw(rawToEng(x))` returns $x$ within floating-point precision limits for Linear and identity calibrations.

### Enum and Formatting
- **Enum Mapping**: Verifies correct bidirectional lookup between integers and strings.
- **Format Consistency**: Validates fixed-precision output and unit suffix concatenation.

## 2. Integration Tests
- **Hierarchical Parameters**: Verifies that `NamedLinearCalibration` correctly retrieves its scale/offset from its `NamedObject` children.
- **Composition**: Tests `CompositeCalibration` by chaining a Linear calibration with a Format calibration.
- **Lua Bindings**: Validates that all calibration types can be instantiated and invoked from Lua scripts.
