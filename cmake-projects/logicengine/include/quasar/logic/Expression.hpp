/**
 * @file Expression.hpp
 * @brief High-performance Lua expression evaluator for logic conditions.
 */

#ifndef QUASAR_LOGIC_EXPRESSION_HPP
#define QUASAR_LOGIC_EXPRESSION_HPP

#include "quasar/named/NamedObject.hpp"
#include <sol/sol.hpp>
#include <string>
#include <memory>
#include <vector>

namespace quasar::logic {

/**
 * @class Expression
 * @brief Encapsulates a Lua expression compiled to bytecode for fast cyclic evaluation.
 */
class Expression {
public:
    Expression(sol::state& lua, const std::string& source);
    Expression() = default;

    /**
     * @brief Evaluates the expression against a root context tree.
     * Uses the global EvaluationPool for safety.
     */
    bool evaluate(const std::shared_ptr<quasar::named::NamedObject>& contextRoot) const;

    bool isEmpty() const { return m_bytecode.empty(); }

    /** @brief Gets the internal bytecode. */
    const std::vector<std::byte>& getBytecode() const { return m_bytecode; }

    /**
     * @brief Binds a NamedObject tree to the Lua state as the global 'ctx'.
     */
    static void bindContext(sol::state& lua, const std::shared_ptr<quasar::named::NamedObject>& contextRoot);

private:
    std::vector<std::byte> m_bytecode;
    sol::state* m_lua{nullptr};
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_EXPRESSION_HPP
