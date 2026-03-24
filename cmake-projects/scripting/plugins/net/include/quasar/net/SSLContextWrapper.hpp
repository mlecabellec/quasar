#ifndef QUASAR_NET_SSLCONTEXTWRAPPER_HPP
#define QUASAR_NET_SSLCONTEXTWRAPPER_HPP

#include "server/asio/ssl_context.h"
#include <sol/sol.hpp>
#include <memory>
#include <string>

namespace quasar::net {

void bindSSLContext(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_SSLCONTEXTWRAPPER_HPP
