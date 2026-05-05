#include <sol/sol.hpp>
#include "quasar/scripting/PluginContract.hpp"
#include "resoem/EthercatMasterService.hpp"
#include "resoem/EthercatSlave.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/IObserver.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"

using namespace quasar::named;
using namespace resoem;
using namespace quasar::scripting;

extern "C" {
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
