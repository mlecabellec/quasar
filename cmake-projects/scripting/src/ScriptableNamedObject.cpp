#include "quasar/scripting/ScriptableNamedObject.hpp"
#include "quasar/scripting/LuaProxy.hpp"

namespace quasar::scripting {

std::shared_ptr<ScriptableNamedObject> ScriptableNamedObject::create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent) {
    struct Enabler : public ScriptableNamedObject {
        explicit Enabler(const std::string& n) : ScriptableNamedObject(n) {}
    };

    std::shared_ptr<ScriptableNamedObject> obj = std::make_shared<Enabler>(name);
    obj->setSelf(obj);
    if (parent) {
        obj->setParent(parent);
    }
    return obj;
}

void ScriptableNamedObject::addChild(std::shared_ptr<named::NamedObject> child) {
    if (getLuaSelf() && getLuaSelf()["onAddChild"].valid()) {
        getLuaSelf()["onAddChild"](getLuaSelf(), LuaProxy<named::NamedObject>(child));
    }
    named::NamedObject::addChild(child);
}

} // namespace quasar::scripting
