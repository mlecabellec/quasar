#include "quasar/scripting/RegistryBindings_Internal.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/NamedLuaMethod.hpp"

namespace quasar::scripting {

using namespace quasar::named;

void bindExtendedTypes(sol::state_view lua, std::shared_ptr<LuaService> service) {
    sol::table quasarTable = lua["quasar"].get_or_create<sol::table>();
    sol::table namedTable = quasarTable["named"].get_or_create<sol::table>();

    // Long
    using NamedLong = NamedInteger<int64_t>;
    sol::usertype<LuaProxy<NamedLong>> utNamedLong = lua.new_usertype<LuaProxy<NamedLong>>("NamedLong", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedLong>(utNamedLong);
    utNamedLong["value"] = [](LuaProxy<NamedLong> self) { return self.lock()->value(); };
    utNamedLong["setValue"] = [](LuaProxy<NamedLong> self, int64_t v) { self.lock()->setValue(v); };
    namedTable.set_function("createLong", [](const std::string& name, int64_t v, sol::object parent, sol::this_state L) -> LuaProxy<NamedLong> {
        std::shared_ptr<NamedLong> ptr = NamedLong::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedLong>(ptr);
    });

    // ULong
    using NamedULong = NamedInteger<uint64_t>;
    sol::usertype<LuaProxy<NamedULong>> utNamedULong = lua.new_usertype<LuaProxy<NamedULong>>("NamedULong", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedULong>(utNamedULong);
    utNamedULong["value"] = [](LuaProxy<NamedULong> self) { return static_cast<int64_t>(self.lock()->value()); };
    utNamedULong["setValue"] = [](LuaProxy<NamedULong> self, int64_t v) { self.lock()->setValue(static_cast<uint64_t>(v)); };
    namedTable.set_function("createULong", [](const std::string& name, int64_t v, sol::object parent, sol::this_state L) -> LuaProxy<NamedULong> {
        std::shared_ptr<NamedULong> ptr = NamedULong::create(name, static_cast<uint64_t>(v), extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedULong>(ptr);
    });

    // Double
    using NamedDouble = NamedFloatingPoint<double>;
    sol::usertype<LuaProxy<NamedDouble>> utNamedDouble = lua.new_usertype<LuaProxy<NamedDouble>>("NamedDouble", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedDouble>(utNamedDouble);
    utNamedDouble["value"] = [](LuaProxy<NamedDouble> self) { return self.lock()->value(); };
    utNamedDouble["setValue"] = [](LuaProxy<NamedDouble> self, double v) { self.lock()->setValue(v); };
    namedTable.set_function("createDouble", [](const std::string& name, double v, sol::object parent, sol::this_state L) -> LuaProxy<NamedDouble> {
        std::shared_ptr<NamedDouble> ptr = NamedDouble::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedDouble>(ptr);
    });

    // Boolean
    sol::usertype<LuaProxy<NamedBoolean>> utNamedBoolean = lua.new_usertype<LuaProxy<NamedBoolean>>("NamedBoolean", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedBoolean>(utNamedBoolean);
    utNamedBoolean["value"] = [](LuaProxy<NamedBoolean> self) { return self.lock()->booleanValue(); };
    utNamedBoolean["setValue"] = [](LuaProxy<NamedBoolean> self, bool v) { self.lock()->setValue(v); };
    namedTable.set_function("createBoolean", [](const std::string& name, bool v, sol::object parent, sol::this_state L) -> LuaProxy<NamedBoolean> {
        std::shared_ptr<NamedBoolean> ptr = NamedBoolean::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedBoolean>(ptr);
    });

    // String
    sol::usertype<LuaProxy<NamedString>> utNamedString = lua.new_usertype<LuaProxy<NamedString>>("NamedString", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedString>(utNamedString);
    utNamedString["value"] = [](LuaProxy<NamedString> self) { return self.lock()->toString(); };
    utNamedString["setValue"] = [](LuaProxy<NamedString> self, const std::string& v) { self.lock()->setValue(v); };
    namedTable.set_function("createString", [](const std::string& name, const std::string& v, sol::object parent, sol::this_state L) -> LuaProxy<NamedString> {
        std::shared_ptr<NamedString> ptr = NamedString::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedString>(ptr);
    });

    // Buffer
    sol::usertype<LuaProxy<NamedBuffer>> utNamedBuffer = lua.new_usertype<LuaProxy<NamedBuffer>>("NamedBuffer", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedBuffer>(utNamedBuffer);
    utNamedBuffer["getSize"] = [](LuaProxy<NamedBuffer> self) { return self.lock()->size(); };
    namedTable.set_function("createBuffer", [](const std::string& name, size_t sz, sol::object parent, sol::this_state L) -> LuaProxy<NamedBuffer> {
        std::shared_ptr<NamedBuffer> ptr = NamedBuffer::create(name, sz, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedBuffer>(ptr);
    });

    // NamedService
    sol::usertype<LuaProxy<NamedService>> utNamedService = lua.new_usertype<LuaProxy<NamedService>>("NamedService", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedService>(utNamedService);
    utNamedService["start"] = [](LuaProxy<NamedService> self) { self.lock()->start(); };
    utNamedService["stop"] = [](LuaProxy<NamedService> self) { self.lock()->stop(); };
    utNamedService["isRunning"] = [](LuaProxy<NamedService> self) { return self.lock()->isRunning(); };
    namedTable.set_function("createService", [](const std::string& name, sol::object parent, sol::this_state L) -> LuaProxy<NamedService> {
        std::shared_ptr<NamedService> ptr = NamedService::create(name, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedService>(ptr);
    });
}

} // namespace quasar::scripting
