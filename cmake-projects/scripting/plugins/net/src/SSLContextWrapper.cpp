#include "quasar/net/SSLContextWrapper.hpp"

namespace quasar::net {

void bindSSLContext(sol::state_view& lua) {
    sol::table netT = lua["quasar"]["net"].get_or_create<sol::table>();
    sol::table securityTable = netT["security"].get_or_create<sol::table>();

    // We expose CppServer::Asio::SSLContext
    // Note: CppServer::Asio::SSLContext is typically created via std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12)
    // We'll wrap its constructor and expose relevant method properties using lambdas.

    // Expose the raw SSL context natively
    securityTable.new_usertype<CppServer::Asio::SSLContext>("SSLContext",
        sol::constructors<CppServer::Asio::SSLContext(asio::ssl::context_base::method)>(),
        "setOptions", [](CppServer::Asio::SSLContext& self, int options) { self.set_options(options); },
        "setPassword", [](CppServer::Asio::SSLContext& self, const std::string& pwd) { self.set_password_callback([pwd](std::size_t, asio::ssl::context_base::password_purpose) { return pwd; }); },
        "useCertificateChainFile", [](CppServer::Asio::SSLContext& self, const std::string& filename) { self.use_certificate_chain_file(filename); },
        "usePrivateKeyFile", [](CppServer::Asio::SSLContext& self, const std::string& filename) { self.use_private_key_file(filename, asio::ssl::context::pem); },
        "useTmpDhFile", [](CppServer::Asio::SSLContext& self, const std::string& filename) { self.use_tmp_dh_file(filename); },
        "loadVerifyFile", [](CppServer::Asio::SSLContext& self, const std::string& filename) { self.load_verify_file(filename); },
        "setVerifyMode", [](CppServer::Asio::SSLContext& self, int mode) { self.set_verify_mode(mode); }
    );

    // Provide enumeration for basic SSL methods since they map to asio::ssl::c    // Context methods enum
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
