/**
 * @file TestDatacodec.cpp
 * @brief Standalone manual verification program for IntegerCodec, ContainerDef, and BitBuffer.
 * @details Exercises schema creation, raw buffer setup, and manual codec decode path against
 * NamedInteger<uint8_t> output.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020
 * @compliance Satisfies CS-0090.1..CS-0090.9 documentation, control-branch annotations, and CS-0010.44 density.
 */
// Standard assertion, stream IO, smart pointer, and vector headers.
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

// Datacodec codecs/schemas and coretypes/named dependencies.
#include "datacodec/BinaryMapper.hpp"
#include "datacodec/ICodec.hpp"
#include "datacodec/IntegerCodec.hpp"
#include "datacodec/Schema.hpp"
// Bit-buffer and NamedInteger headers.
#include "quasar/coretypes/BitBuffer.hpp"
#include "quasar/coretypes/BitBufferSlice.hpp"
#include "quasar/coretypes/Buffer.hpp"
#include "quasar/named/NamedInteger.hpp"

// Bring datacodec, coretypes, and named test symbols into scope.
using namespace datacodec;
using namespace quasar::coretypes;
using namespace quasar::named;

/**
 * @brief Standalone test runner exercising codec schema setup and integer decode verification.
 * @details Creates an 8-bit IntegerCodec, registers a FieldDef on a ContainerDef schema, populates
 * a byte buffer with 0xAB, invokes decode, and validates the decoded NamedInteger<uint8_t> value.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020
 * @compliance Documents entry point return values, error containment, and FE-0020 evaluation notes (CS-0090.1..CS-0090.9).
 * @return Process status code indicating decode verification pass (0) or failure (1).
 *   Justification: Signals manual test pass/fail outcome to shell and CTest runners.
 *   Usage: Returned directly from main to the operating system.
 *   Consumed: Plain integer value consumed by OS process exit handler.
 *   Prerequisites: Runtime environment supports heap allocation and std::cout.
 *   Limitations: Placeholder IntegerCodec decode returns 0 rather than 0xAB if unpacking is incomplete.
 *   Lifecycle: Scalar int scoped to process termination.
 *   Modification / Deletion: Caller does not modify process return code.
 *   Specific Values: 0 when decoded NamedInteger<uint8_t> equals 0xAB; 1 on mismatch or failure.
 *   Error Behavior: Uncaught exceptions from decode/allocation terminate or propagate to runtime.
 * @post Logs test progression and returns 0 on decode match or 1 otherwise.
 * @throws std::exception if BitBuffer, ContainerDef, or IntegerCodec::decode throws.
 * @containment
 *   Approach: Explicit null check on dynamic_pointer_cast prevents null dereference before reading value().
 *   Limitations: No top-level try/catch wraps BitBuffer allocation or decode calls inside standalone test.
 */
int main() {
  std::cout << "Starting Datacodec Test..." << std::endl;

  // 1. Define Codec
  std::cout << "Creating IntegerCodec..." << std::endl;
  std::shared_ptr<IntegerCodec<uint8_t>> uint8Codec =
      std::make_shared<IntegerCodec<uint8_t>>(8);

  // 2. Define Schema
  std::cout << "Creating Schema..." << std::endl;
  std::shared_ptr<ContainerDef> schema = ContainerDef::create("TestContainer");
  std::shared_ptr<FieldDef> field1 = FieldDef::create("Field1", uint8Codec, 0);
  schema->addField(field1);

  // 3. Create Data
  std::cout << "Creating Data..." << std::endl;
  std::vector<uint8_t> rawData = {0xAB};
  std::shared_ptr<BitBuffer> buffer = std::make_shared<BitBuffer>(
      rawData.size() * 8); // Should use rawData construction if available
  // Manually setting byte for now since BitBuffer might not have vector ctor in
  // this context
  buffer->setByte(0, 0xAB);

  std::shared_ptr<BitBuffer> slice = buffer->sliceBits(
      0, 8); // sliceBits returns BitBuffer value, we need view?
  // BitBufferSlice constructor wrapper if needed or usage of shared_ptr
  // Assuming BitBufferSlice has a constructor taking shared_ptr<Buffer>

  // 4. Decode
  // This part requires the BinaryMapper implementation to be complete and
  // linkable. For now, we perform a manual decode check since BinaryMapper is
  // header-only/template.

  std::cout << "Manual Decode Check..." << std::endl;
  std::shared_ptr<NamedObject> decodedObj =
      uint8Codec->decode(slice); // Assuming slice is compatible interface
  std::shared_ptr<NamedInteger<uint8_t>> namedInt =
      std::dynamic_pointer_cast<NamedInteger<uint8_t>>(decodedObj);

  // Verify decoded object is a NamedInteger<uint8_t> equal to 0xAB, returning failure otherwise.
  if (namedInt && namedInt->value() == 0xAB) {
    std::cout << "SUCCESS: Decoded 0xAB correctly!" << std::endl;
  // Report decode failure when cast fails or decoded byte differs from 0xAB.
  } else {
    std::cout << "FAILURE: Failed to decode." << std::endl;
    return 1;
  }

  // Analysis of tests for FE-0020 contribution:
  // The current tests are basic and primarily focus on the IntegerCodec's ability
  // to produce a NamedInteger.
  //
  // FE-0020.4: The test successfully demonstrates the creation of a
  // `quasar::named::NamedInteger<uint8_t>`, confirming that codecs can produce
  // derived NamedObject classes as required.
  //
  // FE-0020.1 (Name validation, parent/child management): The tests do not verify
  // name validation (e.g., non-empty string, uniqueness within parent) or the
  // establishment of parent/child relationships. The `NamedInteger` is created
  // with an empty name ("").
  //
  // FE-0020.2 (Comparison), FE-0020.12 (Traversal), FE-0020.13 (Search): These
  // advanced features of the NamedObject hierarchy are not exercised or verified
  // by the current test sequence.
  //
  // Thus, the implemented test sequence provides only partial proof of contribution
  // and conformance to FE-0020.
  // TO BE CONFIRMED: Full test coverage for FE-0020 features, including name
  // validation, hierarchy management, traversal, and search utilities.
  // TO BE CONFIRMED: Verification of NamedObject name properties (non-empty, unique).

  return 0;
}

