# Calibration Classes

This directory contains the implementation details of the calibration and value transformation framework.

## Core Interfaces
- [Calibrations](Calibrations.md): Definition of `ICalibration` and its basic mathematical implementations (`Linear`, `Polynomial`, `Table`, `Enum`, `Format`).

## Hierarchical Calibrations
- [NamedCalibration](NamedCalibration.md): Integration of calibrations into the Quasar object hierarchy, enabling parameter persistence and tree-based management.
