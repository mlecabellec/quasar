# SMP Module Tests

This document provides a detailed overview of the testing suite for the `smp` module, located in `cmake-projects/smp/test/`. These tests verify the implementation of the ECSS SMP (Simulation Model Portability) standard interfaces and core logic.

The test suite is built using **GoogleTest** (implied, though manual checks seen in code) and/or standalone C++ checks, defined in `CMakeLists.txt`.

## 1. Structure Tests (`StructureTest.cpp`)

**Test Cases:**

*   **`Uuids`**
    *   **Pre-conditions:** None.
    *   **Sequence:** Access `Smp::Uuids::Uuid_Uuid` and `Smp::Uuids::Uuid_Void`.
    *   **Post-conditions:** They are different. `Uuid_Uuid` matches its expected value.

*   **`PrimitiveTypeKind`**
    *   **Pre-conditions:** None.
    *   **Sequence:** Initialize a `PrimitiveTypeKind` variable.
    *   **Post-conditions:** Value is correctly stored and printable.

---

## 2. Core Interfaces Tests (`CoreInterfacesTest.cpp`)

**Test Cases:**

*   **`Compilation`**
    *   **Pre-conditions:** All headers included.
    *   **Sequence:** Compile and run.
    *   **Post-conditions:** Successful compilation proves headers are self-contained.

*   **`Constants`**
    *   **Pre-conditions:** Service interfaces.
    *   **Sequence:** Check static constant names (e.g., `ILogger::SMP_Logger`).
    *   **Post-conditions:** Constants match expected string values (e.g., "Logger", "TimeKeeper").

---

## 3. Publication Tests (`PublicationTest.cpp`)

**Test Cases:**

*   **`PrimitiveTypeRegistration`**
    *   **Pre-conditions:** `TypeRegistry` initialized.
    *   **Sequence:** Query `GetType` for `PTK_Int32` and `Uuid_Int32`.
    *   **Post-conditions:** Returns valid type pointers.

*   **`CustomIntegerType`**
    *   **Pre-conditions:** Registry.
    *   **Sequence:** `AddIntegerType("MyInt", ...)` with a custom UUID.
    *   **Post-conditions:** `GetType(uuid)` returns the registered type.

*   **`PublishField`**
    *   **Pre-conditions:** `Publication` object initialized. Local variable `myVar`.
    *   **Sequence:** `PublishField("MyVar", ..., &myVar, ...)`.
    *   **Post-conditions:** `GetFields()` collection contains "MyVar".

*   **`FieldValueAccess`**
    *   **Pre-conditions:** Published field "MyVar" linked to `myVar` (42).
    *   **Sequence:**
        1. Check `GetValue()` -> 42.
        2. Change `myVar = 100`. Check `GetValue()` -> 100.
        3. `SetValue(200)`. Check `myVar` -> 200.
    *   **Post-conditions:** Field updates reflect on memory and vice-versa.

*   **`CompositeTypes`**
    *   **Pre-conditions:** Registry.
    *   **Sequence:** Register Enumeration, Structure, and Array types.
    *   **Post-conditions:** Types are successfully registered and retrievable.

---

## 4. Simulator Tests (`SimulatorTest.cpp`)

**Test Cases:**

*   **`InitialState`**
    *   **Pre-conditions:** Simulator created.
    *   **Sequence:** `GetState()`.
    *   **Post-conditions:** Returns `SSK_Building`.

*   **`ContainerAccess`**
    *   **Pre-conditions:** Simulator created.
    *   **Sequence:** `GetContainer("Models")`.
    *   **Post-conditions:** Returns valid container.

*   **`StateTransitions`**
    *   **Pre-conditions:** Simulator in `Building`.
    *   **Sequence:**
        1. `Connect()` -> State becomes `Standby`.
        2. `Run()` -> State becomes `Executing`.
        3. `Hold()` -> State becomes `Standby`.
    *   **Post-conditions:** State transitions follow the SMP lifecycle.

---

## 5. All Headers Test (`AllHeadersTest.cpp`)

**Test Cases:**

*   **`IncludeAll`**
    *   **Pre-conditions:** None.
    *   **Sequence:** Include every single `.h` file in `smp/include/Smp`.
    *   **Post-conditions:** Compiles without error. Verifies no missing forward declarations or circular dependencies in headers.
