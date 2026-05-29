/**
 * @file RawEthernetServiceBindings.cpp
 * @brief Lua bindings for RawEthernetService and its properties.
 * 
 * **Compliance**:
 * - Fulfills [CS-0010.34] No auto.
 * - Fulfills [CS-0010.44] Comments on all code blocks.
 * - Fulfills [CS-0010.45] Doxygen comments on all functions.
 * 
 * @feature TSK-20260529-001 Raw Ethernet Socket Service.
 * @exposed
 */

#include "quasar/named/RawEthernetService.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <stdexcept>

namespace quasar::net {

using namespace quasar::named;
using namespace quasar::scripting;

/**
 * @brief Binds RawEthernetService and NamedInteger<uint16_t> to Lua.
 * @param lua The Lua state view.
 */
void bindRawEthernetService(sol::state_view& lua) {
    // [CS-0010.44] Retrieve the 'net' table under the 'quasar' table.
    sol::table netTable = lua["quasar"]["net"].get_or_create<sol::table>();

    // [CS-0010.44] Define the usertype for NamedInteger<uint16_t> to support etherType value/setValue.
    using NamedEtherType = NamedInteger<uint16_t>;
    sol::usertype<LuaProxy<NamedEtherType>> utEtherType = lua.new_usertype<LuaProxy<NamedEtherType>>("NamedEtherType",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    // [CS-0010.44] Map etherType value getter.
    utEtherType["value"] = [](LuaProxy<NamedEtherType>& self) -> uint16_t {
        return self.lock()->value();
    };

    // [CS-0010.44] Map etherType value setter.
    utEtherType["setValue"] = [](LuaProxy<NamedEtherType>& self, uint16_t v) -> void {
        self.lock()->setValue(v);
    };

    // [CS-0010.44] Map etherType getName.
    utEtherType["getName"] = [](LuaProxy<NamedEtherType>& self) -> std::string {
        return self.lock()->getName();
    };

    // [CS-0010.44] Map etherType getType.
    utEtherType["getType"] = [](LuaProxy<NamedEtherType>& self) -> std::string {
        return self.lock()->getType();
    };

    // [CS-0010.44] Register the usertype for RawEthernetService LuaProxy.
    sol::usertype<LuaProxy<RawEthernetService>> ut = lua.new_usertype<LuaProxy<RawEthernetService>>("RawEthernetService",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    // [CS-0010.44] Map service start lifecycle method.
    ut["start"] = [](LuaProxy<RawEthernetService>& self) -> void {
        self.lock()->start();
    };

    // [CS-0010.44] Map service stop lifecycle method with mutex release protection.
    ut["stop"] = [](LuaProxy<RawEthernetService>& self, sol::this_state L) -> void {
        std::shared_ptr<RawEthernetService> svc = self.lock();
        sol::state_view luaState(L);
        sol::object engineObj = luaState["__quasar_engine"];
        
        // [CS-0010.44] Safely unlock Lua mutex before executing blocking stop call.
        if (engineObj.is<std::weak_ptr<LuaEngine>>()) {
            std::shared_ptr<LuaEngine> engine = engineObj.as<std::weak_ptr<LuaEngine>>().lock();
            if (engine) {
                engine->getMutex().unlock();
                svc->stop();
                engine->getMutex().lock();
                return;
            }
        }
        svc->stop();
    };

    // [CS-0010.44] Map isRunning check.
    ut["isRunning"] = [](LuaProxy<RawEthernetService>& self) -> bool {
        return self.lock()->isRunning();
    };

    // [CS-0010.44] Map send method to transmit the outgoing raw frame.
    ut["send"] = [](LuaProxy<RawEthernetService>& self) -> void {
        self.lock()->sendFrame();
    };

    // [CS-0010.44] Map addRule method to register incoming parsing rules.
    ut["addRule"] = [](LuaProxy<RawEthernetService>& self, sol::object ruleObj) -> void {
        if (ruleObj.is<quasar::named::traversal::TransformationRule>()) {
            quasar::named::traversal::TransformationRule rule = ruleObj.as<quasar::named::traversal::TransformationRule>();
            self.lock()->addRule(rule);
        } else {
            throw std::runtime_error("addRule expects a TransformationRule object");
        }
    };

    // [CS-0010.44] Map getName from NamedObject.
    ut["getName"] = [](LuaProxy<RawEthernetService>& self) -> std::string {
        return self.lock()->getName();
    };

    // [CS-0010.44] Map getType from NamedObject.
    ut["getType"] = [](LuaProxy<RawEthernetService>& self) -> std::string {
        return self.lock()->getType();
    };

    // [CS-0010.44] Map interfaceName property to obtain its NamedString proxy.
    ut["interfaceName"] = sol::property([](LuaProxy<RawEthernetService>& self) -> LuaProxy<NamedString> {
        std::shared_ptr<NamedString> node = self.lock()->getInterfaceNameNode();
        return LuaProxy<NamedString>(node);
    });

    // [CS-0010.44] Map etherType property to obtain its NamedEtherType proxy.
    ut["etherType"] = sol::property([](LuaProxy<RawEthernetService>& self) -> LuaProxy<NamedEtherType> {
        std::shared_ptr<NamedEtherType> node = self.lock()->getEtherTypeNode();
        return LuaProxy<NamedEtherType>(node);
    });

    // [CS-0010.44] Map incomingFrame property to obtain its NamedBuffer proxy.
    ut["incomingFrame"] = sol::property([](LuaProxy<RawEthernetService>& self) -> LuaProxy<NamedBuffer> {
        std::shared_ptr<NamedBuffer> node = self.lock()->getIncomingFrameNode();
        return LuaProxy<NamedBuffer>(node);
    });

    // [CS-0010.44] Map outgoingFrame property to obtain its NamedBuffer proxy.
    ut["outgoingFrame"] = sol::property([](LuaProxy<RawEthernetService>& self) -> LuaProxy<NamedBuffer> {
        std::shared_ptr<NamedBuffer> node = self.lock()->getOutgoingFrameNode();
        return LuaProxy<NamedBuffer>(node);
    });

    // [CS-0010.44] Map incomingTree property to obtain its NamedObject proxy.
    ut["incomingTree"] = sol::property([](LuaProxy<RawEthernetService>& self) -> LuaProxy<NamedObject> {
        std::shared_ptr<NamedObject> node = self.lock()->getIncomingTreeNode();
        return LuaProxy<NamedObject>(node);
    });

    // [CS-0010.44] Register the factory method 'new' on quasar.net.RawEthernetService.
    netTable["RawEthernetService"] = lua.create_table_with(
        "new", [](const std::string& name, sol::object parent, sol::this_state L) -> LuaProxy<RawEthernetService> {
            std::shared_ptr<RawEthernetService> ptr = RawEthernetService::create(name, extractNamedObject(parent));
            if (ptr != nullptr && ptr->getParent() == nullptr) {
                ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            }
            return LuaProxy<RawEthernetService>(ptr);
        }
    );
}

} // namespace quasar::net
