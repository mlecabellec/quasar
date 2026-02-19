#pragma once

#include <Smp/IModel.h>
#include <Smp/IService.h>
#include <Smp/PrimitiveTypes.h>

namespace smp_ext {

constexpr const char *DATA_CONNECTION_SERVICE_NAME = "dataConnectionService";

/**
 * This service provides connection data utils for SMP models.
 */
class IDataConnectionService : public virtual Smp::IService {
public:
  virtual ~IDataConnectionService() = default;

  /**
   * Connect one data to another one.
   * @param sourceModel source connected model
   * @param sourceField source model field name
   * @param destModel destination connected model
   * @param destField destination model field name
   */
  virtual void connectData(Smp::IModel *sourceModel, Smp::String8 sourceField,
                           Smp::IModel *destModel, Smp::String8 destField) = 0;

  /**
   * Partially connect one data to another one.
   * @param sourceModel source connected model
   * @param sourceField source model field name
   * @param destModel destination connected model
   * @param destField destination model field name
   * @param destinationOffset connection offset in destination data
   * @param length length of connection
   * @param offset connection offset in source data
   */
  virtual void connectData(Smp::IModel *sourceModel, Smp::String8 sourceField,
                           Smp::IModel *destModel, Smp::String8 destField,
                           Smp::UInt64 destinationOffset, Smp::UInt64 length,
                           Smp::UInt64 offset) = 0;

  /**
   * Indicates if given port of model is connected to another one.
   * @param model source connected model
   * @param field source model field name
   * @return true if connected.
   */
  virtual bool isConnected(Smp::IModel *model, Smp::String8 field) = 0;
};

} // namespace smp_ext
