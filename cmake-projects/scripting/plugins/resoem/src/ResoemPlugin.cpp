#include <sol/sol.hpp>
#include "quasar/scripting/PluginContract.hpp"
#include "resoem/EthercatMasterService.hpp"
#include "resoem/EthercatSlave.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/IObserver.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"

using namespace quasar::named;
using namespace resoem;
using namespace quasar::scripting;

extern "C" {
/// @brief Register components for the Resoem plugin with the sol/Lua state.
/// @param lua The Lua state view.
QUASAR_PLUGIN_EXPORT void registerPluginComponents(sol::state_view lua) {
    sol::table quasar_tbl = lua["quasar"].get_or_create<sol::table>();
    sol::table resoem_tbl = quasar_tbl["resoem"].get_or_create<sol::table>();

    // Bind EthercatMasterService
    sol::usertype<LuaProxy<EthercatMasterService>> utMaster = resoem_tbl.new_usertype<LuaProxy<EthercatMasterService>>("EthercatMasterService",
        sol::base_classes, sol::bases<ILuaProxy>(),
        "create", [](const std::string& name, sol::object parent, sol::this_state L) {
            std::shared_ptr<NamedObject> p = extractNamedObject(parent);
            std::shared_ptr<EthercatMasterService> ptr = EthercatMasterService::create(name, p);
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<EthercatMasterService>(ptr);
        },
        "setInterface", [](LuaProxy<EthercatMasterService>& proxy, const std::string& iface) {
            if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) s->setInterface(iface);
        }
    );
    
    utMaster["start"] = [](LuaProxy<EthercatMasterService>& proxy) { if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) s->start(); };
    utMaster["stop"] = [](LuaProxy<EthercatMasterService>& proxy) { if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) s->stop(); };
    utMaster["getName"] = [](LuaProxy<EthercatMasterService>& proxy) { return proxy.lock() ? proxy.lock()->getName() : ""; };
    utMaster["getChild"] = [](LuaProxy<EthercatMasterService>& proxy, const std::string& n) {
        if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) return LuaProxy<NamedObject>(s->getChild(n));
        return LuaProxy<NamedObject>(nullptr);
    };
    utMaster["subscribe"] = [](LuaProxy<EthercatMasterService>& proxy, std::shared_ptr<IObserver> obs) {
        if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) s->subscribe(obs);
    };
    utMaster["unsubscribe"] = [](LuaProxy<EthercatMasterService>& proxy, std::shared_ptr<IObserver> obs) {
        if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) s->unsubscribe(obs);
    };

    // Configure cable redundancy
    utMaster["setRedundancy"] = [](LuaProxy<EthercatMasterService>& proxy, bool enable, sol::optional<std::string> secondary_iface) {
        if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) {
            std::string sec = secondary_iface.value_or("");
            s->setRedundancy(enable, sec);
        } else {
            // Master service not locked
        }
    };

    // Load and apply an ENI configuration file
    utMaster["loadEni"] = [](LuaProxy<EthercatMasterService>& proxy, const std::string& filepath, sol::this_state L) -> LuaProxy<NamedObject> {
        if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) {
            std::shared_ptr<NamedObject> args = NamedObject::create("args");
            NamedString::create("filepath", filepath, args);
            std::shared_ptr<NamedObject> res = s->loadEni(args);
            if (res != nullptr && !res->getParent()) {
                ObjectTracker::getInstance().trackStrong(getEngineId(L), res);
            } else {
                // Already attached
            }
            return LuaProxy<NamedObject>(res);
        } else {
            return LuaProxy<NamedObject>(nullptr);
        }
    };

    // Export bus topology to an ENI XML file
    utMaster["exportEni"] = [](LuaProxy<EthercatMasterService>& proxy, const std::string& filepath, sol::optional<std::string> masterName, sol::this_state L) -> LuaProxy<NamedObject> {
        if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) {
            std::shared_ptr<NamedObject> args = NamedObject::create("args");
            NamedString::create("filepath", filepath, args);
            if (masterName.has_value()) {
                NamedString::create("masterName", masterName.value(), args);
            } else {
                // Default master name
            }
            std::shared_ptr<NamedObject> res = s->exportEni(args);
            if (res != nullptr && !res->getParent()) {
                ObjectTracker::getInstance().trackStrong(getEngineId(L), res);
            } else {
                // Already attached
            }
            return LuaProxy<NamedObject>(res);
        } else {
            return LuaProxy<NamedObject>(nullptr);
        }
    };

    // Compare live bus or loaded configuration with an ENI reference
    utMaster["compareEni"] = [](LuaProxy<EthercatMasterService>& proxy, const std::string& filepath, sol::optional<std::string> level, sol::this_state L) -> LuaProxy<NamedObject> {
        if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) {
            std::shared_ptr<NamedObject> args = NamedObject::create("args");
            NamedString::create("filepath", filepath, args);
            if (level.has_value()) {
                NamedString::create("level", level.value(), args);
            } else {
                // Default level
            }
            std::shared_ptr<NamedObject> res = s->compareEni(args);
            if (res != nullptr && !res->getParent()) {
                ObjectTracker::getInstance().trackStrong(getEngineId(L), res);
            } else {
                // Already attached
            }
            return LuaProxy<NamedObject>(res);
        } else {
            return LuaProxy<NamedObject>(nullptr);
        }
    };

    // Refresh slave status
    utMaster["refreshStatus"] = [](LuaProxy<EthercatMasterService>& proxy, sol::this_state L) -> LuaProxy<NamedObject> {
        if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) {
            std::shared_ptr<NamedObject> res = s->refreshStatus(nullptr);
            if (res != nullptr && !res->getParent()) {
                ObjectTracker::getInstance().trackStrong(getEngineId(L), res);
            } else {
                // Already attached
            }
            return LuaProxy<NamedObject>(res);
        } else {
            return LuaProxy<NamedObject>(nullptr);
        }
    };

    // Force slaves to INIT
    utMaster["forceInit"] = [](LuaProxy<EthercatMasterService>& proxy, sol::this_state L) -> LuaProxy<NamedObject> {
        if (std::shared_ptr<EthercatMasterService> s = proxy.lock()) {
            std::shared_ptr<NamedObject> res = s->forceInit(nullptr);
            if (res != nullptr && !res->getParent()) {
                ObjectTracker::getInstance().trackStrong(getEngineId(L), res);
            } else {
                // Already attached
            }
            return LuaProxy<NamedObject>(res);
        } else {
            return LuaProxy<NamedObject>(nullptr);
        }
    };

    // Bind EthercatSlave
    sol::usertype<LuaProxy<EthercatSlave>> utSlave = resoem_tbl.new_usertype<LuaProxy<EthercatSlave>>("EthercatSlave",
        sol::base_classes, sol::bases<ILuaProxy>()
    );
    utSlave["getName"] = [](LuaProxy<EthercatSlave>& proxy) { return proxy.lock() ? proxy.lock()->getName() : ""; };
    utSlave["getChild"] = [](LuaProxy<EthercatSlave>& proxy, const std::string& n) {
        if (std::shared_ptr<EthercatSlave> s = proxy.lock()) return LuaProxy<NamedObject>(s->getChild(n));
        return LuaProxy<NamedObject>(nullptr);
    };
    utSlave["subscribe"] = [](LuaProxy<EthercatSlave>& proxy, std::shared_ptr<IObserver> obs) {
        if (std::shared_ptr<EthercatSlave> s = proxy.lock()) s->subscribe(obs);
    };
    utSlave["unsubscribe"] = [](LuaProxy<EthercatSlave>& proxy, std::shared_ptr<IObserver> obs) {
        if (std::shared_ptr<EthercatSlave> s = proxy.lock()) s->unsubscribe(obs);
    };
}
}
