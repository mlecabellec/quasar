# Test Case: `NamedBitBufferSliceTest.DeepCopyRebasing`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedSlices.cpp` (lines 241–288) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedBitBufferSliceTest.DeepCopyRebasing` defined in `cmake-projects/named/test/TestNamedSlices.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
participant "NamedBitBuffer" as SUT1
participant "NamedBitBufferSlice" as SUT2
participant "BitBufferSlice" as SUT3
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : NamedBitBuffer::create( bbuf , 20, root)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : NamedBitBufferSlice::create( slice1 , buf, 2, 10, buf)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : NamedBitBufferSlice::create( slice2 , buf, 5, 5, root)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : NamedBitBufferSlice::create( subSlice , slice1, 3, 4, slice1)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedObject> clonedRoot = root->deepCopy()
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(clonedSlice1->quasar::coretypes::BitBufferSlice::getPar...
Runner -> Runner : EXPECT_NE(clonedSlice1->quasar::coretypes::BitBufferSlice::getPar...
Runner -> Runner : EXPECT_EQ(clonedSlice2->quasar::coretypes::BitBufferSlice::getPar...
Runner -> Runner : EXPECT_EQ(clonedSubSlice->quasar::coretypes::BitBufferSlice::getP...
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `NamedBitBuffer::create("bbuf", 20, root)`
- `NamedBitBufferSlice::create("slice1", buf, 2, 10, buf)`
- `NamedBitBufferSlice::create("slice2", buf, 5, 5, root)`
- `NamedBitBufferSlice::create("subSlice", slice1, 3, 4, slice1)`
- `std::shared_ptr<NamedObject> clonedRoot = root->deepCopy()`
- `std::dynamic_pointer_cast<NamedBitBuffer>(clonedRoot->getFirstChild())`
- `clonedRoot->getLastChild())`

## Verification Assertions

- `EXPECT_EQ(clonedSlice1->quasar::coretypes::BitBufferSlice::getParent(),`
- `EXPECT_NE(clonedSlice1->quasar::coretypes::BitBufferSlice::getParent(), buf)`
- `EXPECT_EQ(clonedSlice2->quasar::coretypes::BitBufferSlice::getParent(), buf)`
- `EXPECT_EQ(clonedSubSlice->quasar::coretypes::BitBufferSlice::getParent(),`
- `EXPECT_EQ(clonedSubSlice->getOffset(), 5)`

---

[Back to `named` Tests Overview](../named.md)
