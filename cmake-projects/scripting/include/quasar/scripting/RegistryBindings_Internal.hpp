#ifndef QUASAR_SCRIPTING_REGISTRYBINDINGS_INTERNAL_HPP
#define QUASAR_SCRIPTING_REGISTRYBINDINGS_INTERNAL_HPP

#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedService.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedArray.hpp"
#include "quasar/named/NamedMap.hpp"
#include <memory>
#include <vector>
#include <list>
#include <optional>

namespace quasar::scripting {

template<typename T, typename U>
void bindBaseMethods(U& ut) {
    ut["getName"] = [](LuaProxy<T> self) { return self.lock()->getName(); };
    ut["setName"] = [](LuaProxy<T> self, const std::string& name) { self.lock()->setName(name); };
    ut["getType"] = [](LuaProxy<T> self) { return self.lock()->getType(); };
    ut["isAlive"] = [](LuaProxy<T> self) { return self.isAlive(); };
    ut["getParent"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<quasar::named::NamedObject>> {
        std::shared_ptr<quasar::named::NamedObject> p = self.lock()->getParent();
        return p ? std::make_optional(LuaProxy<quasar::named::NamedObject>(p)) : std::nullopt;
    };
    ut["getChild"] = [](LuaProxy<T> self, const std::string& name) -> std::optional<LuaProxy<quasar::named::NamedObject>> {
        std::shared_ptr<quasar::named::NamedObject> c = self.lock()->getChild(name);
        return c ? std::make_optional(LuaProxy<quasar::named::NamedObject>(c)) : std::nullopt;
    };
    ut["getChildren"] = [](LuaProxy<T> self) {
        std::list<std::shared_ptr<quasar::named::NamedObject>> children = self.lock()->getChildren();
        std::vector<LuaProxy<quasar::named::NamedObject>> proxies;
        proxies.reserve(children.size());
        for (auto const& c : children) proxies.emplace_back(c);
        return proxies;
    };

    // Common casting helpers
    ut["asLong"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<quasar::named::NamedInteger<int64_t>>> {
        auto p = std::dynamic_pointer_cast<quasar::named::NamedInteger<int64_t>>(self.lock());
        return p ? std::make_optional(LuaProxy<quasar::named::NamedInteger<int64_t>>(p)) : std::nullopt;
    };
    ut["asULong"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<quasar::named::NamedInteger<uint64_t>>> {
        auto p = std::dynamic_pointer_cast<quasar::named::NamedInteger<uint64_t>>(self.lock());
        return p ? std::make_optional(LuaProxy<quasar::named::NamedInteger<uint64_t>>(p)) : std::nullopt;
    };
    ut["asDouble"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<quasar::named::NamedFloatingPoint<double>>> {
        auto p = std::dynamic_pointer_cast<quasar::named::NamedFloatingPoint<double>>(self.lock());
        return p ? std::make_optional(LuaProxy<quasar::named::NamedFloatingPoint<double>>(p)) : std::nullopt;
    };
    ut["asBoolean"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<quasar::named::NamedBoolean>> {
        auto p = std::dynamic_pointer_cast<quasar::named::NamedBoolean>(self.lock());
        return p ? std::make_optional(LuaProxy<quasar::named::NamedBoolean>(p)) : std::nullopt;
    };
    ut["asString"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<quasar::named::NamedString>> {
        auto p = std::dynamic_pointer_cast<quasar::named::NamedString>(self.lock());
        return p ? std::make_optional(LuaProxy<quasar::named::NamedString>(p)) : std::nullopt;
    };
    ut["asService"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<quasar::named::NamedService>> {
        auto p = std::dynamic_pointer_cast<quasar::named::NamedService>(self.lock());
        return p ? std::make_optional(LuaProxy<quasar::named::NamedService>(p)) : std::nullopt;
    };
    ut["asBuffer"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<quasar::named::NamedBuffer>> {
        auto p = std::dynamic_pointer_cast<quasar::named::NamedBuffer>(self.lock());
        return p ? std::make_optional(LuaProxy<quasar::named::NamedBuffer>(p)) : std::nullopt;
    };
}

} // namespace quasar::scripting

#endif // QUASAR_SCRIPTING_REGISTRYBINDINGS_INTERNAL_HPP
