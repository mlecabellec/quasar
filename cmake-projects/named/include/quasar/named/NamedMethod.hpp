#ifndef QUASAR_NAMED_NAMEDMETHOD_HPP
#define QUASAR_NAMED_NAMEDMETHOD_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/ICommand.hpp"
#include <functional>

namespace quasar::named {

/**
 * @class NamedMethod
 * @brief Represents an executable method within the NamedObject hierarchy.
 * 
 * NamedMethod allows encapsulating logic as a node in the tree.
 * It can be discovered and executed dynamically.
 * 
 * @reference [TSK-20260328-001] Reflexive Execution & Service Orchestration
 * @reference [FE-0260.1] Reflexive Methods (NamedMethod)
 */
class NamedMethod : public NamedObject, public ICommand {
public:
    /**
     * @brief Signature for the method implementation.
     * @param owner The containing object (parent of the method).
     * @param args The input arguments as a NamedObject tree.
     * @return The execution result as a NamedObject tree.
     */
    using MethodType = std::function<std::shared_ptr<NamedObject>(std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args)>;

    /**
     * @brief Factory method to create a new NamedMethod.
     * @param name The name of the method.
     * @param method The lambda implementation.
     * @param parent Optional parent to attach the method to.
     * @return A shared_ptr to the newly created NamedMethod.
     */
    static std::shared_ptr<NamedMethod> create(const std::string& name, MethodType method, std::shared_ptr<NamedObject> parent = nullptr);

    /**
     * @brief Executes the method.
     * @param args Shared pointer to a NamedObject containing the arguments.
     * @return Shared pointer to a NamedObject containing the result.
     * @compliance [FE-0130.4.3] Executing bound methods dynamically.
     */
    std::shared_ptr<NamedObject> execute(std::shared_ptr<NamedObject> args) override;

    /**
     * @brief Returns the type of the object as a string.
     * @return "NamedMethod".
     */
    std::string getType() const override;

    /**
     * @brief Creates a standalone copy of this object.
     * @param policy The copy policy.
     * @return Shared pointer to a new NamedMethod with the same name and logic.
     */
    std::shared_ptr<NamedObject> clone(CopyPolicy policy = CopyPolicy::DUPLICATE) const override;

protected:
    /**
     * @brief Protected constructor.
     * @param name The name of the method.
     * @param method The method implementation.
     */
    NamedMethod(const std::string& name, MethodType method);

protected:
    /** @brief The actual logic to execute. */
    MethodType m_method;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDMETHOD_HPP
