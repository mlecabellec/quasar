# Named Module Tests

This document provides a detailed overview of the testing suite for the `named` module, located in `cmake-projects/named/test/`. These tests ensure the robustness of the hierarchical named object system, including tree management, traversal algorithms, and serialization.

The test suite is built using **GoogleTest** and is defined in `CMakeLists.txt`, which handles dependency fetching (GoogleTest v1.14.0) and test discovery.

## 1. NamedObject Tests (`TestNamedObject.cpp`)

**Test Cases:**

*   **`Creation`**
    *   **Pre-conditions:** None.
    *   **Sequence:** Create object "root".
    *   **Post-conditions:** Name matches "root", parent is `nullptr`.

*   **`InvalidName`**
    *   **Pre-conditions:** None.
    *   **Sequence:** Attempt creation with names "", "123", "invalid-name".
    *   **Post-conditions:** All attempts throw `std::runtime_error`.

*   **`Hierarchy`**
    *   **Pre-conditions:** None.
    *   **Sequence:** Create "root", "child1" (under root). Create "child2". Set "child2" parent to root. Unparent "child2".
    *   **Post-conditions:** Initially, root has 2 children. "child1" -> next sibling is "child2". After unparenting, root has 1 child ("child1").

*   **`Uniqueness`**
    *   **Pre-conditions:** "root" has child "child".
    *   **Sequence:** Attempt to add another child named "child" to "root".
    *   **Post-conditions:** Throws `std::runtime_error`.

*   **`CycleDetection`**
    *   **Pre-conditions:** Chain `p1 -> p2 -> p3`.
    *   **Sequence:** Attempt `p1->setParent(p3)` (cycle) or `p1->setParent(p1)` (self-loop).
    *   **Post-conditions:** Throws `std::runtime_error`.

*   **`DerivedClasses`**
    *   **Pre-conditions:** Root node.
    *   **Sequence:** Create `NamedInteger` and `NamedBoolean` children.
    *   **Post-conditions:** Child values are correctly stored and retrieved.

*   **`SerializationXML` / `SerializationYAML`**
    *   **Pre-conditions:** Tree `root -> val` (NamedInteger).
    *   **Sequence:** Call `toXml(root)` and `toYaml(root)`.
    *   **Post-conditions:** Output strings contain expected fragments (`name="root"`, `name: root`).

*   **`NamedBuffers`**
    *   **Pre-conditions:** Root node.
    *   **Sequence:** Create `NamedBuffer` and `NamedBitBuffer` objects.
    *   **Post-conditions:** Byte and bit data are preserved and accessible through the named interface.

*   **`RelatedObject`**
    *   **Pre-conditions:** Two `NamedObject` instances.
    *   **Sequence:** Set one as related to the other. Destroy the related object.
    *   **Post-conditions:** Relationship is initially established. After destruction, the weak reference is automatically cleared (returns `nullptr`).

---

## 2. Named Slices Tests (`TestNamedSlices.cpp`)

**Test Cases:**

*   **`NamedBufferSliceTest.CreationAndUsage`**
    *   **Pre-conditions:** `Buffer` with known data.
    *   **Sequence:** Create `NamedBufferSlice`. Create a sub-slice from it.
    *   **Post-conditions:** Slice correctly accesses original buffer data. Sub-slice tracks parent slice and has generated name (e.g., `parent_slice`).

*   **`NamedBitBufferSliceTest.CreationAndUsage`**
    *   **Pre-conditions:** `BitBuffer` with known bits.
    *   **Sequence:** Create `NamedBitBufferSlice`. Create a sub-slice.
    *   **Post-conditions:** Slice and sub-slice access correct bit ranges and report correct sizes.

*   **`NamedBufferSliceTest.Clone`**
    *   **Pre-conditions:** `NamedBufferSlice` pointing to buffer data.
    *   **Sequence:** Call `clone()`.
    *   **Post-conditions:** Clone is a distinct `NamedBufferSlice` with identical state.

---

## 3. Named String Tests (`TestNamedString.cpp`)

**Test Cases:**

*   **`CreationAndValue`**
    *   **Pre-conditions:** None.
    *   **Sequence:** Create `NamedString` with name and initial content.
    *   **Post-conditions:** Name, content, and length are correctly reported.

*   **`ParentChild`**
    *   **Pre-conditions:** Root node.
    *   **Sequence:** Create `NamedString` as a child of root.
    *   **Post-conditions:** Parent-child relationship is correctly established in both directions.

*   **`Clone`**
    *   **Pre-conditions:** `NamedString` with content.
    *   **Sequence:** Call `clone()`.
    *   **Post-conditions:** Clone has identical name and content.

---

## 4. Stress Tests (`TestNamedObjectStress.cpp`)

**Test Cases:**

*   **`DeepHierarchy`**
    *   **Pre-conditions:** Root node.
    *   **Sequence:** Iteratively create children 1000 levels deep. Search for "node_999".
    *   **Post-conditions:** Leaf node is found and has correct name. Stack does not overflow.

*   **`WideHierarchy`**
    *   **Pre-conditions:** Root node.
    *   **Sequence:** Create 5000 children under root. Search for "child_4999".
    *   **Post-conditions:** Root child count is 5000. Target child is found.

---

## 5. Traversal Tests (`TestTraversal.cpp`)

**Test Cases:**

*   **`DFS`**
    *   **Pre-conditions:** Tree `root -> {c1 -> c11, c2}`.
    *   **Sequence:** Execute `forEachDepthFirst`.
    *   **Post-conditions:** Visits nodes in order: `root`, `c1`, `c11`, `c2`.

*   **`BFS`**
    *   **Pre-conditions:** Tree `root -> {c1 -> c11, c2}`.
    *   **Sequence:** Execute `forEachBreadthFirst`.
    *   **Post-conditions:** Visits nodes in order: `root`, `c1`, `c2`, `c11`.

*   **`DeepCopy`**
    *   **Pre-conditions:** Tree `root -> c1` (NamedInteger 42).
    *   **Sequence:** Call `deepCopy(root)`. Modify copy (add child).
    *   **Post-conditions:** Original tree is unchanged. Copy contains a `NamedInteger` with value 42. Copy structure matches original at time of copy.
