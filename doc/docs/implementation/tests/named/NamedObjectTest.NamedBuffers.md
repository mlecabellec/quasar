# Test Case: `NamedObjectTest.NamedBuffers`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedObject.cpp` (lines 207–232) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedObjectTest.NamedBuffers` defined in `cmake-projects/named/test/TestNamedObject.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
participant "NamedBuffer" as SUT1
participant "NamedBitBuffer" as SUT2
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedBuffer> nb = NamedBuffer::create( buffer , d...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : NamedBitBuffer::create( bitbuffer , 16, root)
SUT2 --> Runner : return / state updated
Runner -> SUT0 : nbb->setBit(0, true)
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(nb->size(), 2)
Runner -> Runner : EXPECT_EQ(nb->get(0), 0x01)
Runner -> Runner : EXPECT_EQ(nbb->bitSize(), 16)
Runner -> Runner : EXPECT_TRUE(nbb->getBit(0))
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `std::shared_ptr<NamedBuffer> nb = NamedBuffer::create("buffer", data, root)`
- `NamedBitBuffer::create("bitbuffer", 16, root)`
- `nbb->setBit(0, true)`

## Verification Assertions

- `EXPECT_EQ(nb->size(), 2)`
- `EXPECT_EQ(nb->get(0), 0x01)`
- `EXPECT_EQ(nbb->bitSize(), 16)`
- `EXPECT_TRUE(nbb->getBit(0))`

---

[Back to `named` Tests Overview](../named.md)
