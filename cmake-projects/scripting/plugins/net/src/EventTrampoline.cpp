#include "quasar/net/EventTrampoline.hpp"

namespace quasar::net {

EventTrampoline& EventTrampoline::getInstance() {
    static EventTrampoline instance;
    return instance;
}

} // namespace quasar::net
