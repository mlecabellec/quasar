#include "quasar/net/SSLContextWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"

namespace quasar::net {

using namespace quasar::scripting;

void bindSSLContext(sol::state_view& lua) {
    sol::table netT = lua["quasar"]["net"].get_or_create<sol::table>();
    sol::table securityTable = netT["security"].get_or_create<sol::table>();

    sol::usertype<LuaProxy<CppServer::Asio::SSLContext>> ut = lua.new_usertype<LuaProxy<CppServer::Asio::SSLContext>>("SSLContext",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    securityTable["SSLContext"] = lua.create_table_with(
        "new", [](asio::ssl::context_base::method m, sol::this_state L) {
            std::shared_ptr<CppServer::Asio::SSLContext> ptr = std::make_shared<CppServer::Asio::SSLContext>(m);
            // SSLContext doesn't inherit NamedObject, but we use trackStrong to bind its lifecycle to the engine.
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<CppServer::Asio::SSLContext>(ptr);
        }
    );

    ut["setOptions"] = [](LuaProxy<CppServer::Asio::SSLContext> self, int options) { self.lock()->set_options(options); };
    ut["setPassword"] = [](LuaProxy<CppServer::Asio::SSLContext> self, const std::string& pwd) { 
        self.lock()->set_password_callback([pwd](std::size_t, asio::ssl::context_base::password_purpose) { return pwd; }); 
    };
    ut["useCertificateChainFile"] = [](LuaProxy<CppServer::Asio::SSLContext> self, const std::string& filename) { self.lock()->use_certificate_chain_file(filename); };
    ut["usePrivateKeyFile"] = [](LuaProxy<CppServer::Asio::SSLContext> self, const std::string& filename) { self.lock()->use_private_key_file(filename, asio::ssl::context::pem); };
    ut["useTmpDhFile"] = [](LuaProxy<CppServer::Asio::SSLContext> self, const std::string& filename) { self.lock()->use_tmp_dh_file(filename); };
    ut["loadVerifyFile"] = [](LuaProxy<CppServer::Asio::SSLContext> self, const std::string& filename) { self.lock()->load_verify_file(filename); };
    ut["addVerifyPath"] = [](LuaProxy<CppServer::Asio::SSLContext> self, const std::string& path) { self.lock()->add_verify_path(path); };
    ut["setVerifyMode"] = [](LuaProxy<CppServer::Asio::SSLContext> self, int mode) { self.lock()->set_verify_mode(mode); };
    ut["setDefaultVerifyPaths"] = [](LuaProxy<CppServer::Asio::SSLContext> self) { self.lock()->set_default_verify_paths(); };

    // Context methods enum
    securityTable["method"] = lua.create_table_with(
        "sslv2", asio::ssl::context_base::sslv2,
        "sslv23", asio::ssl::context::sslv23,
        "tlsv12", asio::ssl::context::tlsv12,
        "tlsv13", asio::ssl::context::tlsv13
    );

    securityTable["verify_mode"] = lua.create_table_with(
        "none", asio::ssl::verify_none,
        "peer", asio::ssl::verify_peer,
        "fail_if_no_peer_cert", asio::ssl::verify_fail_if_no_peer_cert,
        "client_once", asio::ssl::verify_client_once
    );
}

} // namespace quasar::net
