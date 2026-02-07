#ifndef SMP_STORAGE_H
#define SMP_STORAGE_H

#include "Smp/IStorageReader.h"
#include "Smp/IStorageWriter.h"
#include <fstream>
#include <string>

namespace Smp {

class StorageReader : public virtual IStorageReader {
public:
  StorageReader(String8 filename);
  virtual ~StorageReader() noexcept;

  void Restore(void *address, UInt64 size) override;
  String8 GetStateVectorFileName() const override;
  String8 GetStateVectorFilePath() const override;

private:
  std::string filename;
  std::ifstream file;
};

class StorageWriter : public virtual IStorageWriter {
public:
  StorageWriter(String8 filename);
  virtual ~StorageWriter() noexcept;

  void Store(void *address, UInt64 size) override;
  String8 GetStateVectorFileName() const override;
  String8 GetStateVectorFilePath() const override;

private:
  std::string filename;
  std::ofstream file;
};

} // namespace Smp

#endif // SMP_STORAGE_H
