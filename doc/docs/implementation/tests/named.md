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

*   **Serialization (XML / YAML / JSON)**
    - **Pre-conditions**: A complex tree containing `NamedInteger`, `NamedBoolean`, `NamedString`, `NamedFloatingPoint`, `NamedBuffer`, and `NamedBitBuffer`.
    - **Sequence**: Execute round-trip serialization: `toXml`/`fromXml`, `toYaml`/`fromYaml`, `toJson`/`fromJson`.
    - **Post-conditions**: The restored tree must exactly match the original in terms of names, hierarchy, and values for all data types.

---

## 2. Named Slices Tests (`TestNamedSlices.cpp`)

**Test Cases:**

*   **`NamedBufferSliceTest.CreationAndUsage`**
    - **Pre-conditions**: `Buffer` with known data.
    - **Sequence**: Create `NamedBufferSlice`. Create a sub-slice from it.
    - **Post-conditions**: Slice correctly accesses original buffer data. Sub-slice tracks parent slice and has generated name (e.g., `parent_slice`).

*   **`NamedBitBufferSliceTest.CreationAndUsage`**
    - **Pre-conditions**: `BitBuffer` with known bits.
    - **Sequence**: Create `NamedBitBufferSlice`. Create a sub-slice.
    - **Post-conditions**: Slice and sub-slice access correct bit ranges and report correct sizes.

*   **`NamedBufferSliceTest.Clone`**
    - **Pre-conditions**: `NamedBufferSlice` pointing to buffer data.
    - **Sequence**: Call `clone()`.
    - **Post-conditions**: Clone is a distinct `NamedBufferSlice` with identical state.

---

## 3. Named String Tests (`TestNamedString.cpp`)

**Test Cases:**

*   **`CreationAndValue`**
    - **Pre-conditions**: None.
    - **Sequence**: Create `NamedString` with name and initial content.
    - **Post-conditions**: Name, content, and length are correctly reported.

*   **`ParentChild`**
    - **Pre-conditions**: Root node.
    - **Sequence**: Create `NamedString` as a child of root.
    - **Post-conditions**: Parent-child relationship is correctly established in both directions.

*   **`Clone`**
    - **Pre-conditions**: `NamedString` with content.
    - **Sequence**: Call `clone()`.
    - **Post-conditions**: Clone has identical name and content.

---

## 4. Stress Tests (`TestNamedObjectStress.cpp`)

**Test Cases:**

*   **`DeepHierarchy`**
    - **Pre-conditions**: Root node.
    - **Sequence**: Iteratively create children 1000 levels deep. Search for "node_999".
    - **Post-conditions**: Leaf node is found and has correct name. Stack does not overflow.

*   **`WideHierarchy`**
    - **Pre-conditions**: Root node.
    - **Sequence**: Create 5000 children under root. Search for "child_4999".
    - **Post-conditions**: Root child count is 5000. Target child is found.

---

## 5. Traversal Tests (`TestTraversal.cpp`)

**Test Cases:**

*   **`DFS`**
    - **Pre-conditions**: Tree `root -> {c1 -> c11, c2}`.
    - **Sequence**: Execute `forEachDepthFirst`.
    - **Post-conditions**: Visits nodes in order: `root`, `c1`, `c11`, `c2`.

*   **`BFS`**
    - **Pre-conditions**: Tree `root -> {c1 -> c11, c2}`.
    - **Sequence**: Execute `forEachBreadthFirst`.
    - **Post-conditions**: Visits nodes in order: `root`, `c1`, `c2`, `c11`.

*   **`DeepCopy`**
    - **Pre-conditions**: Tree `root -> c1` (NamedInteger 42).
    - **Sequence**: Call `deepCopy(root)`. Modify copy (add child).
    - **Post-conditions**: Original tree is unchanged. Copy contains a `NamedInteger` with value 42. Copy structure matches original at time of copy.

---

## 6. Thread Safety Tests (`TestNamedObjectThreadSafety.cpp`)

The thread safety tests ensure that concurrent operations on the `NamedObject` tree do not cause deadlocks or race conditions.

### Stress Test Case

- **Setup**: One root `NamedObject` shared among multiple threads.
- **Concurrent Actions**:
    - **Adder Threads (2)**: Continually create new child objects with unique IDs.
    - **Remover Threads (2)**: Randomly select and detach children from the parent.
    - **Reader Threads (4)**: Continually iterate over the child list and access object names.
- **Duration**: 1 second of intensive concurrent execution.
- **Verification**: No crashes, assertion failures, or deadlocks occur during the test period. Objects remains in a consistent state throughout the operations.

<!-- CS-0100-CATALOGUE-START -->
## Registered Test Case Pages (`CS-0100.22`)

| Test Case | Documentation Page |
| :--- | :--- |
| `TestActiveEntity.LifecycleTransitions` | [TestActiveEntity.LifecycleTransitions](named/TestActiveEntity.LifecycleTransitions.md) |
| `TestActiveEntity.ObserverPattern` | [TestActiveEntity.ObserverPattern](named/TestActiveEntity.ObserverPattern.md) |
| `TestActiveEntity.FieldReflexivity` | [TestActiveEntity.FieldReflexivity](named/TestActiveEntity.FieldReflexivity.md) |
| `TestActiveEntity.MethodReflexivity` | [TestActiveEntity.MethodReflexivity](named/TestActiveEntity.MethodReflexivity.md) |
| `TestBoundPrimitives.NamedIntegerBinding` | [TestBoundPrimitives.NamedIntegerBinding](named/TestBoundPrimitives.NamedIntegerBinding.md) |
| `TestBoundPrimitives.NamedFloatingPointBinding` | [TestBoundPrimitives.NamedFloatingPointBinding](named/TestBoundPrimitives.NamedFloatingPointBinding.md) |
| `TestBoundPrimitives.NamedBooleanBinding` | [TestBoundPrimitives.NamedBooleanBinding](named/TestBoundPrimitives.NamedBooleanBinding.md) |
| `TestBoundPrimitives.BoundaryCheck` | [TestBoundPrimitives.BoundaryCheck](named/TestBoundPrimitives.BoundaryCheck.md) |
| `TestBoundPrimitives.CloneSharingPolicy` | [TestBoundPrimitives.CloneSharingPolicy](named/TestBoundPrimitives.CloneSharingPolicy.md) |
| `MethodPerformanceTest.CallOverheadComparison` | [MethodPerformanceTest.CallOverheadComparison](named/MethodPerformanceTest.CallOverheadComparison.md) |
| `MethodPerformanceTest.CoreNamedObjectIsolation` | [MethodPerformanceTest.CoreNamedObjectIsolation](named/MethodPerformanceTest.CoreNamedObjectIsolation.md) |
| `MethodPerformanceTest.ValidationRejection` | [MethodPerformanceTest.ValidationRejection](named/MethodPerformanceTest.ValidationRejection.md) |
| `NamedContainersTest.NamedArray` | [NamedContainersTest.NamedArray](named/NamedContainersTest.NamedArray.md) |
| `NamedContainersTest.NamedMap` | [NamedContainersTest.NamedMap](named/NamedContainersTest.NamedMap.md) |
| `NamedContainersTest.NamedSet` | [NamedContainersTest.NamedSet](named/NamedContainersTest.NamedSet.md) |
| `NamedContainersTest.Iterators` | [NamedContainersTest.Iterators](named/NamedContainersTest.Iterators.md) |
| `NamedMethodAndServiceTest.NamedMethodBasic` | [NamedMethodAndServiceTest.NamedMethodBasic](named/NamedMethodAndServiceTest.NamedMethodBasic.md) |
| `NamedMethodAndServiceTest.NamedMethodOwnerAccess` | [NamedMethodAndServiceTest.NamedMethodOwnerAccess](named/NamedMethodAndServiceTest.NamedMethodOwnerAccess.md) |
| `NamedMethodAndServiceTest.NamedServiceLifecycle` | [NamedMethodAndServiceTest.NamedServiceLifecycle](named/NamedMethodAndServiceTest.NamedServiceLifecycle.md) |
| `NamedMethodAndServiceTest.NamedServiceHooks` | [NamedMethodAndServiceTest.NamedServiceHooks](named/NamedMethodAndServiceTest.NamedServiceHooks.md) |
| `NamedMethodAndServiceTest.MultiThreadStress` | [NamedMethodAndServiceTest.MultiThreadStress](named/NamedMethodAndServiceTest.MultiThreadStress.md) |
| `NamedObjectTest.Creation` | [NamedObjectTest.Creation](named/NamedObjectTest.Creation.md) |
| `NamedObjectTest.InvalidName` | [NamedObjectTest.InvalidName](named/NamedObjectTest.InvalidName.md) |
| `NamedObjectTest.Hierarchy` | [NamedObjectTest.Hierarchy](named/NamedObjectTest.Hierarchy.md) |
| `NamedObjectTest.Uniqueness` | [NamedObjectTest.Uniqueness](named/NamedObjectTest.Uniqueness.md) |
| `NamedObjectTest.CycleDetection` | [NamedObjectTest.CycleDetection](named/NamedObjectTest.CycleDetection.md) |
| `NamedObjectTest.DerivedClasses` | [NamedObjectTest.DerivedClasses](named/NamedObjectTest.DerivedClasses.md) |
| `NamedObjectTest.SerializationXML` | [NamedObjectTest.SerializationXML](named/NamedObjectTest.SerializationXML.md) |
| `NamedObjectTest.SerializationYAML` | [NamedObjectTest.SerializationYAML](named/NamedObjectTest.SerializationYAML.md) |
| `NamedObjectTest.NamedBuffers` | [NamedObjectTest.NamedBuffers](named/NamedObjectTest.NamedBuffers.md) |
| `NamedObjectTest.RelatedObject` | [NamedObjectTest.RelatedObject](named/NamedObjectTest.RelatedObject.md) |
| `NamedObjectTest.RTTI` | [NamedObjectTest.RTTI](named/NamedObjectTest.RTTI.md) |
| `NamedObjectTest.ReplaceInTree` | [NamedObjectTest.ReplaceInTree](named/NamedObjectTest.ReplaceInTree.md) |
| `NamedObjectStress.DeepHierarchy` | [NamedObjectStress.DeepHierarchy](named/NamedObjectStress.DeepHierarchy.md) |
| `NamedObjectStress.WideHierarchy` | [NamedObjectStress.WideHierarchy](named/NamedObjectStress.WideHierarchy.md) |
| `NamedObjectThreadSafety.StressTest` | [NamedObjectThreadSafety.StressTest](named/NamedObjectThreadSafety.StressTest.md) |
| `NamedBufferSliceTest.CreationAndUsage` | [NamedBufferSliceTest.CreationAndUsage](named/NamedBufferSliceTest.CreationAndUsage.md) |
| `NamedBufferSliceTest.Clone` | [NamedBufferSliceTest.Clone](named/NamedBufferSliceTest.Clone.md) |
| `NamedBufferSliceTest.OutOfBoundsTests` | [NamedBufferSliceTest.OutOfBoundsTests](named/NamedBufferSliceTest.OutOfBoundsTests.md) |
| `NamedBufferSliceTest.DeepCopyRebasing` | [NamedBufferSliceTest.DeepCopyRebasing](named/NamedBufferSliceTest.DeepCopyRebasing.md) |
| `NamedBitBufferSliceTest.CreationAndUsage` | [NamedBitBufferSliceTest.CreationAndUsage](named/NamedBitBufferSliceTest.CreationAndUsage.md) |
| `NamedBitBufferSliceTest.OutOfBoundsTests` | [NamedBitBufferSliceTest.OutOfBoundsTests](named/NamedBitBufferSliceTest.OutOfBoundsTests.md) |
| `NamedBitBufferSliceTest.DeepCopyRebasing` | [NamedBitBufferSliceTest.DeepCopyRebasing](named/NamedBitBufferSliceTest.DeepCopyRebasing.md) |
| `NamedStringTest.CreationAndValue` | [NamedStringTest.CreationAndValue](named/NamedStringTest.CreationAndValue.md) |
| `NamedStringTest.ParentChild` | [NamedStringTest.ParentChild](named/NamedStringTest.ParentChild.md) |
| `NamedStringTest.Clone` | [NamedStringTest.Clone](named/NamedStringTest.Clone.md) |
| `NamedVariantTest.BasicUsage` | [NamedVariantTest.BasicUsage](named/NamedVariantTest.BasicUsage.md) |
| `NamedVariantTest.Cloning` | [NamedVariantTest.Cloning](named/NamedVariantTest.Cloning.md) |
| `NamedVariantsAndTemporalTest.NamedVariant` | [NamedVariantsAndTemporalTest.NamedVariant](named/NamedVariantsAndTemporalTest.NamedVariant.md) |
| `NamedVariantsAndTemporalTest.CollectionVariants` | [NamedVariantsAndTemporalTest.CollectionVariants](named/NamedVariantsAndTemporalTest.CollectionVariants.md) |
| `NamedVariantsAndTemporalTest.TemporalTimestampTest` | [NamedVariantsAndTemporalTest.TemporalTimestampTest](named/NamedVariantsAndTemporalTest.TemporalTimestampTest.md) |
| `NamedVariantsAndTemporalTest.TemporalDurationTest` | [NamedVariantsAndTemporalTest.TemporalDurationTest](named/NamedVariantsAndTemporalTest.TemporalDurationTest.md) |
| `NamedVariantsAndTemporalTest.TemporalDateTest` | [NamedVariantsAndTemporalTest.TemporalDateTest](named/NamedVariantsAndTemporalTest.TemporalDateTest.md) |
| `PredefinedRulesTest.SliceBufferRuleShare` | [PredefinedRulesTest.SliceBufferRuleShare](named/PredefinedRulesTest.SliceBufferRuleShare.md) |
| `PredefinedRulesTest.ExtractIntegerRule` | [PredefinedRulesTest.ExtractIntegerRule](named/PredefinedRulesTest.ExtractIntegerRule.md) |
| `ProtocolMappingTest.CastToStructureZeroCopy` | [ProtocolMappingTest.CastToStructureZeroCopy](named/ProtocolMappingTest.CastToStructureZeroCopy.md) |
| `ProtocolMappingTest.ExtractIntegerZeroCopy` | [ProtocolMappingTest.ExtractIntegerZeroCopy](named/ProtocolMappingTest.ExtractIntegerZeroCopy.md) |
| `TestRawEthernetService.VerifyCreationAndPropertyMapping` | [TestRawEthernetService.VerifyCreationAndPropertyMapping](named/TestRawEthernetService.VerifyCreationAndPropertyMapping.md) |
| `TestRawEthernetService.VerifyPermissionSkippingOnStart` | [TestRawEthernetService.VerifyPermissionSkippingOnStart](named/TestRawEthernetService.VerifyPermissionSkippingOnStart.md) |
| `TestRawEthernetService.VerifyIncomingTreeMaterialization` | [TestRawEthernetService.VerifyIncomingTreeMaterialization](named/TestRawEthernetService.VerifyIncomingTreeMaterialization.md) |
| `SerializationTest.XMLRoundTrip` | [SerializationTest.XMLRoundTrip](named/SerializationTest.XMLRoundTrip.md) |
| `SerializationTest.YAMLRoundTrip` | [SerializationTest.YAMLRoundTrip](named/SerializationTest.YAMLRoundTrip.md) |
| `SerializationTest.JSONRoundTrip` | [SerializationTest.JSONRoundTrip](named/SerializationTest.JSONRoundTrip.md) |
| `SerializationTest.BinaryRoundTrip` | [SerializationTest.BinaryRoundTrip](named/SerializationTest.BinaryRoundTrip.md) |
| `TransformerTest.BasicOutwardTransformation` | [TransformerTest.BasicOutwardTransformation](named/TransformerTest.BasicOutwardTransformation.md) |
| `TransformerTest.InPlaceTransformation` | [TransformerTest.InPlaceTransformation](named/TransformerTest.InPlaceTransformation.md) |
| `TransformerTest.ComplexTreeTransformation` | [TransformerTest.ComplexTreeTransformation](named/TransformerTest.ComplexTreeTransformation.md) |
| `TransformerTest.RecursionLimit` | [TransformerTest.RecursionLimit](named/TransformerTest.RecursionLimit.md) |
| `TransformerTest.RecursiveModification` | [TransformerTest.RecursiveModification](named/TransformerTest.RecursiveModification.md) |
| `TransformerTest.RulePriority` | [TransformerTest.RulePriority](named/TransformerTest.RulePriority.md) |
| `TransformerStressTest.ConcurrentInPlaceTransformation` | [TransformerStressTest.ConcurrentInPlaceTransformation](named/TransformerStressTest.ConcurrentInPlaceTransformation.md) |
| `TransformerStressTest.PerformanceBenchmark` | [TransformerStressTest.PerformanceBenchmark](named/TransformerStressTest.PerformanceBenchmark.md) |
| `TraversalTest.DFS` | [TraversalTest.DFS](named/TraversalTest.DFS.md) |
| `TraversalTest.BFS` | [TraversalTest.BFS](named/TraversalTest.BFS.md) |
| `TraversalTest.FindByName` | [TraversalTest.FindByName](named/TraversalTest.FindByName.md) |
| `TraversalTest.DeepCopy` | [TraversalTest.DeepCopy](named/TraversalTest.DeepCopy.md) |
| `NamedTutorialTest.Tutorial1_SimpleNamedPrimitivesAndQuantities` | [NamedTutorialTest.Tutorial1_SimpleNamedPrimitivesAndQuantities](named/NamedTutorialTest.Tutorial1_SimpleNamedPrimitivesAndQuantities.md) |
| `NamedTutorialTest.Tutorial2_IntermediateReflexiveAndWebNamedMethod` | [NamedTutorialTest.Tutorial2_IntermediateReflexiveAndWebNamedMethod](named/NamedTutorialTest.Tutorial2_IntermediateReflexiveAndWebNamedMethod.md) |
| `NamedTutorialTest.Tutorial3_AdvancedTreeTransformationAndSerialization` | [NamedTutorialTest.Tutorial3_AdvancedTreeTransformationAndSerialization](named/NamedTutorialTest.Tutorial3_AdvancedTreeTransformationAndSerialization.md) |
| `NamedCoverageTest.ConfigConstantsCopyPoliciesVariantAndErrorBranches` | [NamedCoverageTest.ConfigConstantsCopyPoliciesVariantAndErrorBranches](named/NamedCoverageTest.ConfigConstantsCopyPoliciesVariantAndErrorBranches.md) |
| `NamedFuzzStressTest.TransformationFuzzAndConcurrentNamedVariantStress` | [NamedFuzzStressTest.TransformationFuzzAndConcurrentNamedVariantStress](named/NamedFuzzStressTest.TransformationFuzzAndConcurrentNamedVariantStress.md) |
