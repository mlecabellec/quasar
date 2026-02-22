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

  return 0;
}
