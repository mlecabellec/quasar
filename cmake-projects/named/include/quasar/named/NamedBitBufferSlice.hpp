#pragma once

#include "quasar/coretypes/BitBufferSlice.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

class NamedBitBufferSlice : public NamedObject,
                            public quasar::coretypes::BitBufferSlice {
public:
  static std::shared_ptr<NamedBitBufferSlice>
  create(const std::string &name,
         std::shared_ptr<quasar::coretypes::BitBuffer> buffer, size_t startBit,
         size_t bitLength, std::shared_ptr<NamedObject> parent = nullptr);

  NamedBitBufferSlice(const std::string &name,
                      std::shared_ptr<quasar::coretypes::BitBuffer> buffer,
                      size_t startBit, size_t bitLength);

  virtual ~NamedBitBufferSlice() = default;

  std::shared_ptr<NamedObject> clone() const override;

  std::shared_ptr<NamedBitBufferSlice> sliceView(size_t startBit,
                                                 size_t bitLength) const;
};

} // namespace quasar::named
