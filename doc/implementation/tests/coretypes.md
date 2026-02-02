# Coretypes Module Tests

This document provides a detailed overview of the testing suite for the `coretypes` module, located in `cmake-projects/coretypes/test/`. These tests ensure the reliability, correctness, and performance of the fundamental data types used throughout the Quasar project.

The test suite is built using **GoogleTest** and is defined in `CMakeLists.txt`, which handles dependency fetching (GoogleTest v1.14.0) and test discovery.

## 1. BitBuffer Tests

Tests for the `BitBuffer` class are split across three files, covering basic functionality, slicing/views, and stress testing.

### Basic Functionality (`TestBitBuffer.cpp`)

**Test Cases:**

*   **`GetSetBit`**
    *   **Pre-conditions:** A `BitBuffer` of 16 bits (2 bytes) is allocated.
    *   **Sequence:** Execute `setBit(0, true)` followed by `setBit(15, true)`.
    *   **Post-conditions:** `getBit(0)` must return `true`. `getBit(1)` must return `false` (default initialization). `getBit(15)` must return `true`.

*   **`BitSize`**
    *   **Pre-conditions:** A `BitBuffer` is initialized with a requested size of 10 bits.
    *   **Sequence:** Query `bitSize()` and `size()` (byte size).
    *   **Post-conditions:** `bitSize()` returns 10. `size()` returns 2 (bytes required to hold 10 bits).

*   **`SliceBits`**
    *   **Pre-conditions:** A `BitBuffer` of 8 bits is set to `0xF0` (11110000).
    *   **Sequence:** Call `sliceBits(2, 4)` to extract 4 bits starting from index 2.
    *   **Post-conditions:** The returned buffer has `bitSize()` 4. The bits (originally indices 2,3,4,5 -> 1,1,0,0) form the byte `0xC0` (11000000) when aligned to MSB.

*   **`ConcatBits`**
    *   **Pre-conditions:** Buffer A (2 bits: `11`) and Buffer B (2 bits: `00`) exist.
    *   **Sequence:** Execute `a.concatBits(b)`.
    *   **Post-conditions:** Resulting buffer has `bitSize()` 4. The bit pattern is `1100`.

*   **`ReverseBits`**
    *   **Pre-conditions:** Buffer (4 bits) initialized to `1100`.
    *   **Sequence:** Execute `reverseBits()`.
    *   **Post-conditions:** Buffer content becomes `0011`.

*   **`ReverseBitsGroup`**
    *   **Pre-conditions:** Buffer (6 bits) initialized to `10 11 00`.
    *   **Sequence:** Execute `reverseBits(2)` (group size of 2).
    *   **Post-conditions:** Groups are reversed: `00 11 10`.

*   **`OutOfRange`**
    *   **Pre-conditions:** `BitBuffer` of 8 bits.
    *   **Sequence:** Attempt `getBit(8)`, `setBit(100, true)`, and `sliceBits(5, 5)`.
    *   **Post-conditions:** All calls must throw `std::out_of_range`. `getBit(7)` should succeed.

*   **`Equals`**
    *   **Pre-conditions:** Two `BitBuffer` objects with same size and bit patterns, and one with different size.
    *   **Sequence:** Compare buffers using `equals()`. Modify one and compare again.
    *   **Post-conditions:** Returns true for identical content/size. Returns false if a single bit differs or if sizes differ.

*   **`Clone`**
    *   **Pre-conditions:** `BitBuffer` with specific bits set.
    *   **Sequence:** Call `clone()`. Modify original buffer.
    *   **Post-conditions:** Clone has identical initial state. Modification of original does not affect clone (deep copy).

*   **`Performance_GetSet`**
    *   **Pre-conditions:** `BitBuffer` of 1KB.
    *   **Sequence:** Perform 1,000,000 `setBit` operations in a loop.
    *   **Post-conditions:** Execution time is measured and asserted to be less than 1000ms (sanity check).

*   **`ThreadSafety`**
    *   **Pre-conditions:** Shared `BitBuffer` (1024 bits).
    *   **Sequence:** Spawn asynchronous writer (continually setting bits) and reader (continually reading bits) for 100ms.
    *   **Post-conditions:** No crashes or race conditions detected (implicit check via TSAN/ASAN in CI).

### Slicing and Views (`TestBitBufferSlice.cpp`)

**Test Cases:**

*   **`CreationAndGet`**
    *   **Pre-conditions:** Shared `BitBuffer` (16 bits) with bits 0 and 15 set to true.
    *   **Sequence:** Create `BitBufferSlice` covering the full range (0, 16).
    *   **Post-conditions:** View size is 16. View correctly reads true at indices 0 and 15.

*   **`SubSlice`**
    *   **Pre-conditions:** Shared `BitBuffer` (16 bits) with bit 2 set to true.
    *   **Sequence:** Create view `slice` (1, 5). Read `slice` index 1. Modify `slice` index 0 to true.
    *   **Post-conditions:** `slice` index 1 corresponds to original index 2 (true). `slice` index 0 corresponds to original index 1 (now true in parent buffer).

*   **`Concat`**
    *   **Pre-conditions:** Two `BitBuffer` objects (4 bits each), `1000` and `0001`.
    *   **Sequence:** Create slices for both. Concatenate the slices.
    *   **Post-conditions:** Result is a new `BitBuffer` of 8 bits with pattern `10000001`.

### Stress Testing (`TestBitBufferStress.cpp`)

**Test Cases:**

*   **`LargeAllocation`**
    *   **Pre-conditions:** System has sufficient memory.
    *   **Sequence:** Allocate 800 Mbits (~100 MB). Set and get the last bit.
    *   **Post-conditions:** Allocation succeeds. Last bit matches set value.

*   **`HeavySlice`**
    *   **Pre-conditions:** 100,000 bit buffer.
    *   **Sequence:** Set every even bit. Slice the first half.
    *   **Post-conditions:** Slice size is 50,000. Slice content preserves the alternating bit pattern.

---

## 2. Boolean Tests (`TestBoolean.cpp`)

**Test Cases:**

*   **`ConstructorString`**
    *   **Pre-conditions:** None.
    *   **Sequence:** Initialize `Boolean` with "true", "TRUE", "false", "random".
    *   **Post-conditions:** "true"/"TRUE" yield true. "false"/"random" yield false.

*   **`FromNumeric`**
    *   **Pre-conditions:** None.
    *   **Sequence:** Call `fromNumeric` with 1, 0, and 3.14.
    *   **Post-conditions:** 1 and 3.14 result in true. 0 results in false.

---

## 3. Buffer Tests (`TestBuffer.cpp`)

**Test Cases:**

*   **`NumericIO`**
    *   **Pre-conditions:** 8-byte Buffer.
    *   **Sequence:** `writeInt(0x12345678, 0, BigEndian)`. `writeInt(0x12345678, 4, LittleEndian)`.
    *   **Post-conditions:** Bytes 0-3 are `12 34 56 78`. Bytes 4-7 are `78 56 34 12`. Reading back with respective endianness returns original integer.

*   **`SliceView`**
    *   **Pre-conditions:** Shared Buffer (10 bytes) initialized with `0x11` at [0] and `0x22` at [1].
    *   **Sequence:** Create view (0, 5). Update parent [0] to `0x33`. Update view [1] to `0x44`.
    *   **Post-conditions:** View [0] reflects `0x33`. Parent [1] reflects `0x44`.

*   **`BitwiseOps`**
    *   **Pre-conditions:** Buffers `b1` ("f0") and `b2` ("0f").
    *   **Sequence:** Compute `b1 & b2`, `b1 | b2`, `b1 ^ b2`, `~b1`.
    *   **Post-conditions:** AND="00", OR="ff", XOR="ff", NOT="0f".

*   **`ComparisonOps`**
    *   **Pre-conditions:** Buffers "aabb", "aabb", "aacc".
    *   **Sequence:** Compare them using `compareTo`.
    *   **Post-conditions:** Identical buffers return 0. "aabb" < "aacc".

---

## 4. Floating Point Tests (`TestFloatingPoint.cpp`)

**Test Cases:**

*   **`SafeArithmetic`**
    *   **Pre-conditions:** `Double` initialized to `max()`, `one`, `zero`.
    *   **Sequence:** Attempt `max * 2`, `1 / 0`, `1 + NaN`.
    *   **Post-conditions:** `safeMultiply` throws `std::overflow_error`. `safeDivide` and `safeAdd` (with NaN) throw `std::runtime_error`.

*   **`StringParsing`**
    *   **Pre-conditions:** Strings "3.14", "1.5".
    *   **Sequence:** Construct `Float`/`Double` from strings.
    *   **Post-conditions:** Values match standard floating point representations (3.14f, 1.5). Invalid strings throw `std::invalid_argument`.

---

## 5. Integer Tests (`TestInteger.cpp`)

**Test Cases:**

*   **`SafeArithmeticOverflow`**
    *   **Pre-conditions:** `Int` initialized to `INT_MAX`.
    *   **Sequence:** Call `safeAdd(1)`.
    *   **Post-conditions:** Throws `std::overflow_error`.

*   **`UByteType`**
    *   **Pre-conditions:** `UByte` 200 and 100.
    *   **Sequence:** Perform standard `add` and `safeAdd`.
    *   **Post-conditions:** Standard `add` wraps to 44. `safeAdd` throws `std::overflow_error`.

*   **`Endianness`**
    *   **Pre-conditions:** `Int` value `0x12345678`.
    *   **Sequence:** Call `swapBytes()`.
    *   **Post-conditions:** Result is `0x78563412`.

*   **`BitwiseOperations`**
    *   **Pre-conditions:** Integers 10 (`1010`) and 12 (`1100`).
    *   **Sequence:** Perform AND, OR, XOR, NOT, and bit shifts.
    *   **Post-conditions:** AND yields 8, OR yields 14, XOR yields 6, NOT(10) yields -11. Left shift 10 by 1 yields 20.

*   **`Introspection`**
    *   **Pre-conditions:** `Int` and `UByte` instances.
    *   **Sequence:** Query `getType()`, `isIntegerType()`, and `isSigned()`.
    *   **Post-conditions:** Both report type "Integer". `Int` is signed; `UByte` is not.

---

## 6. String Tests (`TestString.cpp`)

**Test Cases:**

*   **`Comparison`**
    *   **Pre-conditions:** Strings "abc", "def".
    *   **Sequence:** Check equality and `compareTo`.
    *   **Post-conditions:** `equals` returns false. "abc" compares less than "def".
