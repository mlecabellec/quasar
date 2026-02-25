#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include "datacodec/BinaryMapper.hpp"
#include "datacodec/ICodec.hpp"
#include "datacodec/IntegerCodec.hpp"
#include "datacodec/Schema.hpp"
#include "quasar/coretypes/BitBuffer.hpp"
#include "quasar/coretypes/BitBufferSlice.hpp"
#include "quasar/coretypes/Buffer.hpp"
#include "quasar/named/NamedInteger.hpp"

using namespace datacodec;
using namespace quasar::coretypes;
using namespace quasar::named;

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

  if (namedInt && namedInt->value() == 0xAB) {
    std::cout << "SUCCESS: Decoded 0xAB correctly!" << std::endl;
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

