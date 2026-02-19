# SMP Definition Comparison Report

This report compares the SMP (Simulation Model Portability) definitions found in the **Target** implementation (`cmake-projects/smp`) against two references:
1.  **Old Reference**: `tmp/smp_reference/SMP-2.0` (C++ Mapping v1.2, 2005)
2.  **ECSS Standard**: `tmp/ECSS_SMP_L1` (E-ST-40-07C Rev.1, 2025)

## Executive Summary

The **Target** implementation (`cmake-projects/smp`) is **highly compliant** with the **ECSS E-ST-40-07C Rev.1 (2025)** standard.

*   **Status**: The Target is a modern implementation that matches the 2025 ECSS standard's architectural changes.
*   **Incompatibility with Old Reference**: It is **not backward compatible** with the older SMP 2.0 (2005) mapping due to major structural shifts (moved lifecycle methods, parent access).
*   **Missing Features**: The Target is missing a few minor optional arguments/overloads present in the ECSS 2025 headers (e.g., `LoadLibrary` flags), but strictly adheres to the core interfaces.

---

## 1. Compliance Matrix

| Feature                       | Old Ref (2005)  | ECSS Ref (2025)  | Target (`cmake-projects/smp`) | Status          |
| :---------------------------- | :-------------- | :--------------- | :---------------------------- | :-------------- |
| **`GetParent()`**             | `IComponent`    | `IObject`        | `IObject`                     | ✅ **Compliant** |
| **Lifecycle**                 | `IModel`        | `IComponent`     | `IComponent`                  | ✅ **Compliant** |
| **`GetState()`**              | `IModel`        | `IComponent`     | `IComponent`                  | ✅ **Compliant** |
| **`IModel`**                  | Lifecycle logic | Marker Interface | Marker Interface              | ✅ **Compliant** |
| **`IComponent::Disconnect`**  | ❌               | ✅                | ✅                             | ✅ **Compliant** |
| **`ISimulator::LoadLibrary`** | ❌               | ✅ (w/ Flags)     | ✅ (No Flags)                  | ⚠️ **Partial**   |

---

## 2. Detailed Analysis vs. ECSS E-ST-40-07C Rev.1

### A. `Smp::IObject`
*   **ECSS Standard**:
    *   `GetName()`, `GetDescription()`, `GetParent()`.
    *   **New**: `GetChild(String8 name)`.
*   **Target**:
    *   Matches `GetName`, `GetDescription`, `GetParent`.
    *   ❌ **Missing**: `GetChild(String8 name)`.
*   **Impact**: Minor. `GetChild` is useful for traversal but often not critical for basic model interoperability if `GetFields`/`GetContainer` logic is used.

### B. `Smp::IComponent`
*   **ECSS Standard**:
    *   Inherits `IObject`.
    *   Lifecycle: `Publish`, `Configure`, `Connect`, `Disconnect`, `GetState`.
    *   Field Access: `GetField`, `GetFields`.
    *   Value Access: `GetSimpleValue`, `SetSimpleValue`, `GetSimpleArrayValue`, `SetSimpleArrayValue`.
    *   Child Management: `AddChild`, `RemoveChild`, `IsChildInCollection`.
    *   `GetUuid`.
*   **Target**:
    *   Matches Lifecycle methods perfectly (`Publish`, `Configure`, `Connect`, `Disconnect`, `GetState`).
    *   Matches `GetField`, `GetFields`, `GetUuid`.
    *   ❌ **Missing**: Value Access methods (`GetSimpleValue`, etc.) directly on `IComponent` (might be relying on `IField` interfaces).
    *   ❌ **Missing**: Child Management methods (`AddChild`, `RemoveChild`, `IsChildInCollection`).
*   **Impact**: **Moderate**. The missing Child Management methods might affect generic container implementations or dynamic composition features if client code relies on `IComponent` for composition rather than `IComposite`. The missing Value Access convenience methods force users to go through `IField`.

### C. `Smp::IModel`
*   **ECSS Standard**: Marker interface (empty body or destructor only).
*   **Target**: Marker interface.
*   **Status**: ✅ **Fully Compliant**.

### D. `Smp::ISimulator`
*   **ECSS Standard**:
    *   Lifecycle, Model/Service Management (`Add/GetModel`, `Add/GetService`).
    *   Factory Management (`RegisterFactory`, `CreateInstance`, `GetFactory`, `GetFactories`).
    *   Type Registry (`GetTypeRegistry`).
    *   Library Loading (`LoadLibrary(path, flag)`).
*   **Target**:
    *   Matches almost all methods signature-for-signature.
    *   ⚠️ **Difference**: `LoadLibrary(String8 libraryPath)` in Target vs `LoadLibrary(String8 libraryPath, LibraryLoadingFlag flag)` in ECSS. The Target lacks the second argument.
*   **Status**: ✅ **Compliant (mostly)**. The `LoadLibrary` signature difference is the only notable API mismatch.

---

## 3. Conclusion

The `cmake-projects/smp` project is clearly intended to implement the **ECSS E-ST-40-07C** standard (likely an earlier draft or a slight variation of the final 2025 release).

**Incompatibilities with Reference (ECSS 2025):**
1.  **Missing `IObject::GetChild`**: Code relying on generic object tree traversal via names will fail.
2.  **Missing `IComponent` Child Management**: `AddChild`/`RemoveChild` are absent. This suggests the Target might handle composition strictly through `IComposite` or specific container interfaces, whereas ECSS 2025 generalizes this on `IComponent` (likely for `IComposite` to implement).
3.  **Missing `IComponent` Simple Value Access**: Convenience methods for setting values by string path are missing.
4.  **`LoadLibrary` Signature**: Missing the `LibraryLoadingFlag`.

**Recommendation**:
To achieve strict ECSS E-ST-40-07C compliance, the following changes are recommended for `cmake-projects/smp`:
1.  Add `GetChild` to `IObject`.
2.  Add `AddChild`, `RemoveChild`, `IsChildInCollection` to `IComponent`.
3.  Add `GetSimpleValue`, `SetSimpleValue`, `GetSimpleArrayValue`, `SetSimpleArrayValue` to `IComponent`.
4.  Update `LoadLibrary` in `ISimulator` to accept the `LibraryLoadingFlag`.
