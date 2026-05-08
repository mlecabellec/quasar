/**
 * @file ZmqBindings.cpp
 * @brief Lua bindings for ZeroMQ plugin.
 */

#include "quasar/zmq/Socket.hpp"
#include "quasar/zmq/Context.hpp"
#include "quasar/scripting/PluginContract.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include <sol/sol.hpp>

namespace {
// Helper removed, now using quasar::scripting::extractNamedObject
} // namespace

extern "C" {
    /**
     * @brief Registers ZeroMQ components into the Lua environment.
     * 
     * Fulfills [TSK-20260311-004.1.1] Expose ZMQ context and socket types to Lua.
     * Fulfills [TSK-20260311-004.5.2] Organize under quasar.zmq namespace.
     * 
     * @param lua State view.
     */
    QUASAR_PLUGIN_EXPORT void registerPluginComponents(sol::state_view lua) {
        // [CS-0010.34] auto forbidden.
        sol::table quasar = lua["quasar"].get_or_create<sol::table>();
        sol::table zmq = quasar["zmq"].get_or_create<sol::table>();

        // Bind Context
        zmq.new_usertype<quasar::zmq::Context>("Context",
            sol::meta_function::construct, sol::factories([]() {
                return std::make_unique<quasar::zmq::Context>();
            }),
            "socket", [](quasar::zmq::Context& self, int type) {
                return std::make_unique<quasar::zmq::Socket>(self, type);
            }
        );

        // Bind Socket
        zmq.new_usertype<quasar::zmq::Socket>("Socket",
            sol::no_constructor,
            "bind", &quasar::zmq::Socket::bind,
            "connect", &quasar::zmq::Socket::connect,
            "subscribe", &quasar::zmq::Socket::subscribe,
            "unsubscribe", &quasar::zmq::Socket::unsubscribe,
            "send", [](quasar::zmq::Socket& self, const std::string& msg, sol::optional<int> flags) {
                self.send(msg, flags.value_or(0));
            },
            "receive", [](quasar::zmq::Socket& self, sol::optional<int> flags) -> std::optional<std::string> {
                return self.receive(flags.value_or(0));
            },
            "publishTree", [](quasar::zmq::Socket& self, const std::string& topic, sol::object treeObj) {
                std::shared_ptr<quasar::named::NamedObject> root = quasar::scripting::extractNamedObject(treeObj);
                if (root == nullptr) {
                    throw std::runtime_error("Argument to publishTree must be a NamedObject proxy");
                }
                self.publishTree(topic, root);
            },
            "receiveTree", [](quasar::zmq::Socket& self, sol::optional<int> flags, sol::this_state L) -> sol::object {
                std::shared_ptr<quasar::named::NamedObject> root = self.receiveTree(flags.value_or(0));
                if (root != nullptr) {
                    quasar::scripting::ObjectTracker::getInstance().trackStrong(quasar::scripting::getEngineId(L), root);
                    return sol::make_object(L, quasar::scripting::LuaProxy<quasar::named::NamedObject>(root));
                }
                return sol::make_object(L, sol::nil);
            }
        );

        // Constants
        zmq["PUB"] = ZMQ_PUB;
        zmq["SUB"] = ZMQ_SUB;
        zmq["DONTWAIT"] = ZMQ_DONTWAIT;
        zmq["SNDMORE"] = ZMQ_SNDMORE;
        
        // Additional socket types
        zmq["PUSH"] = ZMQ_PUSH;
        zmq["PULL"] = ZMQ_PULL;
        zmq["REQ"] = ZMQ_REQ;
        zmq["REP"] = ZMQ_REP;
    }
}
