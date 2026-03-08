#ifndef QUASAR_NAMED_ICOMMAND_HPP
#define QUASAR_NAMED_ICOMMAND_HPP

#include <memory>
#include <string>

namespace quasar::named {

class NamedObject;

/**
 * @class ICommand
 * @brief Interface for reflexive method execution.
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    /**
     * @brief Executes the command with the given argument.
     * @param args Shared pointer to a NamedObject containing the arguments.
     * @return Shared pointer to a NamedObject containing the result.
     */
    virtual std::shared_ptr<NamedObject> execute(std::shared_ptr<NamedObject> args) = 0;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_ICOMMAND_HPP
