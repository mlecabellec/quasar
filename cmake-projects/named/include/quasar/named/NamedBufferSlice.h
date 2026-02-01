#pragma once

#include "quasar/coretypes/BufferSlice.hpp"
#include "quasar/named/NamedObject.h"

namespace quasar::named {

class NamedBufferSlice : public NamedObject,
                         public quasar::coretypes::BufferSlice {
public:
  static std::shared_ptr<NamedBufferSlice>
  create(const std::string &name,
         std::shared_ptr<quasar::coretypes::Buffer> buffer, size_t start,
         size_t length, std::shared_ptr<NamedObject> parent = nullptr);

  NamedBufferSlice(const std::string &name,
                   std::shared_ptr<quasar::coretypes::Buffer> buffer,
                   size_t start, size_t length);

  virtual ~NamedBufferSlice() = default;

  std::shared_ptr<NamedObject> clone() const override;

  std::shared_ptr<NamedBufferSlice> sliceView(size_t start,
                                              size_t length) const;
};

} // namespace quasar::named
