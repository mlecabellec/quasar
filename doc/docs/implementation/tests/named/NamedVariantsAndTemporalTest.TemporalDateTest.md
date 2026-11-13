# Test Case: `NamedVariantsAndTemporalTest.TemporalDateTest`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedVariantsAndTemporal.cpp` (lines 103–113) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verifies the functionality of the NamedDate class.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedDate" as SUT0
Runner -> SUT0 : std::shared_ptr<NamedDate> date = NamedDate::create( date , 19000)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::string iso = date->toISO8601()
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(date->value(), 19000)
Runner -> Runner : EXPECT_EQ(date->getType(), NamedDate )
Runner -> Runner : EXPECT_TRUE(std::regex_match(iso, iso_regex))
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedDate> date = NamedDate::create("date", 19000)`
- `std::string iso = date->toISO8601()`

## Verification Assertions

- `EXPECT_EQ(date->value(), 19000)`
- `EXPECT_EQ(date->getType(), "NamedDate")`
- `EXPECT_TRUE(std::regex_match(iso, iso_regex))`

---

[Back to `named` Tests Overview](../named.md)
