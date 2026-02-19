#pragma once

namespace smp_ext {

/**
 * Interface of an activator of smp models.
 */
class IModelActivator {
public:
  virtual ~IModelActivator() = default;

  /**
   * Immediately activate the associated model.
   * @return true if successful.
   */
  virtual bool activate() const = 0;
};

} // namespace smp_ext
