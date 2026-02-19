#pragma once

#include "smp_ext/IModelActivator.hpp"
#include <Smp/IEntryPoint.h>
#include <Smp/IService.h>

namespace smp_ext {

constexpr const char *ACTIVATION_SERVICE_NAME = "activationService";

/**
 * This service provides methods to activate an entry point immediately.
 */
class IActivationService : public virtual Smp::IService {
public:
  virtual ~IActivationService() = default;

  /**
   * Activate immediately an entry point.
   * @param ep The entry point.
   */
  virtual void activateImmediatly(const Smp::IEntryPoint *ep) = 0;

  /**
   * Create a model activator that permits the immediate activation of given
   * entry point.
   * @param ep The entry point.
   * @return The model activator.
   */
  virtual IModelActivator *getModelActivator(const Smp::IEntryPoint &ep) = 0;
};

} // namespace smp_ext
