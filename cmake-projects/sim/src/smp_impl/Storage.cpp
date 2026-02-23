#include "Smp/Storage.h"
#include "Smp/CannotRestore.h"
#include "Smp/CannotStore.h"

namespace Smp {

StorageReader::StorageReader(String8 filename)
    : filename(filename ? filename : "") {
  file.open(this->filename, std::ios::binary);
  if (!file.is_open()) {
    // throw CannotRestore(); // Basic implementation
  }
}

StorageReader::~StorageReader() noexcept {
  if (file.is_open())
    file.close();
}

void StorageReader::Restore(void *address, UInt64 size) {
  if (!file.is_open() || !address)
    return;
  file.read(static_cast<char *>(address), size);
}

String8 StorageReader::GetStateVectorFileName() const {
  return filename.c_str();
}
String8 StorageReader::GetStateVectorFilePath() const {
  return filename.c_str();
}

StorageWriter::StorageWriter(String8 filename)
    : filename(filename ? filename : "") {
  file.open(this->filename, std::ios::binary);
  if (!file.is_open()) {
    // throw CannotStore();
  }
}

StorageWriter::~StorageWriter() noexcept {
  if (file.is_open())
    file.close();
}

void StorageWriter::Store(void *address, UInt64 size) {
  if (!file.is_open() || !address)
    return;
  file.write(static_cast<const char *>(address), size);
}

String8 StorageWriter::GetStateVectorFileName() const {
  return filename.c_str();
}
String8 StorageWriter::GetStateVectorFilePath() const {
  return filename.c_str();
}

} // namespace Smp
