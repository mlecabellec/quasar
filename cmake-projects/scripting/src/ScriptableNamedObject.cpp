#include "quasar/scripting/ScriptableNamedObject.hpp"

namespace quasar::scripting {

std::shared_ptr<ScriptableNamedObject> ScriptableNamedObject::create(const std::string& name, std::shared_ptr<named::NamedObject> parent) {
    struct Enabler : public ScriptableNamedObject {
        Enabler(const std::string& n) : ScriptableNamedObject(n) {}
    };
    std::shared_ptr<ScriptableNamedObject> obj = std::make_shared<Enabler>(name);
    obj->setSelf(obj);
    if (parent) {
        obj->setParent(parent);
    }
    return obj;
}

} // namespace quasar::scripting
