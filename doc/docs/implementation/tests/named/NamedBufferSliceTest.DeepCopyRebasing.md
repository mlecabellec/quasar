# Test Case: `NamedBufferSliceTest.DeepCopyRebasing`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedSlices.cpp` (lines 193–239) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedBufferSliceTest.DeepCopyRebasing` defined in `cmake-projects/named/test/TestNamedSlices.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
participant "NamedBuffer" as SUT1
participant "NamedBufferSlice" as SUT2
participant "BufferSlice" as SUT3
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedBuffer> buf = NamedBuffer::create( buf , 20,...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : NamedBufferSlice::create( slice1 , buf, 2, 10, buf)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : NamedBufferSlice::create( slice2 , buf, 5, 5, root)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : NamedBufferSlice::create( subSlice , slice1, 3, 4, slice1)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedObject> clonedRoot = root->deepCopy()
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(clonedSlice1->quasar::coretypes::BufferSlice::getParent(),
Runner -> Runner : EXPECT_NE(clonedSlice1->quasar::coretypes::BufferSlice::getParent...
Runner -> Runner : EXPECT_EQ(clonedSlice2->quasar::coretypes::BufferSlice::getParent...
Runner -> Runner : EXPECT_EQ(clonedSubSlice->quasar::coretypes::BufferSlice::getPare...
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `std::shared_ptr<NamedBuffer> buf = NamedBuffer::create("buf", 20, root)`
- `NamedBufferSlice::create("slice1", buf, 2, 10, buf)`
- `NamedBufferSlice::create("slice2", buf, 5, 5, root)`
- `NamedBufferSlice::create("subSlice", slice1, 3, 4, slice1)`
- `std::shared_ptr<NamedObject> clonedRoot = root->deepCopy()`
- `std::dynamic_pointer_cast<NamedBuffer>(clonedRoot->getFirstChild())`
- `std::dynamic_pointer_cast<NamedBufferSlice>(clonedRoot->getLastChild())`

## Verification Assertions

- `EXPECT_EQ(clonedSlice1->quasar::coretypes::BufferSlice::getParent(),`
- `EXPECT_NE(clonedSlice1->quasar::coretypes::BufferSlice::getParent(), buf)`
- `EXPECT_EQ(clonedSlice2->quasar::coretypes::BufferSlice::getParent(), buf)`
- `EXPECT_EQ(clonedSubSlice->quasar::coretypes::BufferSlice::getParent(),`
- `EXPECT_EQ(clonedSubSlice->getOffset(), 5)`

---

[Back to `named` Tests Overview](../named.md)
