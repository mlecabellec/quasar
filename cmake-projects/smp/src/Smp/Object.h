#ifndef SMP_OBJECT_H
#define SMP_OBJECT_H

#include "Smp/IObject.h"
#include <string>

namespace Smp {
class Object : public virtual IObject {
public:
  Object(String8 name, String8 description, IObject *parent)
      : name(name ? name : ""), description(description ? description : ""),
        parent(parent) {}
  virtual ~Object() noexcept = default;

  String8 GetName() const override { return name.c_str(); }
  String8 GetDescription() const override { return description.c_str(); }
  IObject *GetParent() const override { return parent; }

protected:
  std::string name;
  std::string description;
  IObject *parent;
};
} // namespace Smp
#endif // SMP_OBJECT_H
