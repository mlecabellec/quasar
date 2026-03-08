#include "quasar/scripting/ScriptableNamedObject.hpp"

namespace quasar::scripting {

std::shared_ptr<ScriptableNamedObject> ScriptableNamedObject::create(const std::string& name, std::shared_ptr<named::NamedObject> parent) {
    auto obj = std::shared_ptr<ScriptableNamedObject>(new ScriptableNamedObject(name));
    obj->setSelf(obj);
    if (parent) {
        obj->setParent(parent);
    }
    return obj;
}

} // namespace quasar::scripting
