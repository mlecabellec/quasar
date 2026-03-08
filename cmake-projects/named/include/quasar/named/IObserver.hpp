#ifndef QUASAR_NAMED_IOBSERVER_HPP
#define QUASAR_NAMED_IOBSERVER_HPP

#include <memory>
#include <string>

namespace quasar::named {

class NamedObject;

/**
 * @class IObserver
 * @brief Interface for observing events or state changes from an ActiveEntity or similar producers.
 */
class IObserver {
public:
    virtual ~IObserver() = default;

    /**
     * @brief Called by the producer to notify the observer of an event.
     * @param eventData Shared pointer containing the event payload.
     */
    virtual void notify(std::shared_ptr<NamedObject> eventData) = 0;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_IOBSERVER_HPP
