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
 * 
 * **Compliance**:
 * - [CS-0010.34] No auto.
 * - [TSK-20260311-009.5] Integration & Scripting.
 */
class Expression {
public:
    /**
     * @brief Constructs an expression from Lua source code.
     * @param lua Reference to the sol::state.
     * @param source Lua code (e.g., "ctx.temperature > 50").
     */
    Expression(sol::state& lua, const std::string& source);

    /**
     * @brief Default constructor for empty/always-true expressions.
     */
    Expression() = default;

    /**
     * @brief Evaluates the expression against a root context tree.
     * @param contextRoot The root NamedObject representing 'ctx' in Lua.
     * @return Boolean result of the expression.
     */
    bool evaluate(const std::shared_ptr<quasar::named::NamedObject>& contextRoot) const;

    /**
     * @brief Checks if the expression is empty (always returns true).
     */
    bool isEmpty() const { return m_bytecode.empty(); }

private:
    /** @brief Compiled bytecode of the expression. */
    std::vector<std::byte> m_bytecode;
    /** @brief Lua state used for evaluation. */
    sol::state* m_lua{nullptr};

    /**
     * @brief Recursively maps a NamedObject tree to a Lua table.
     * @param node Current NamedObject node.
     * @param table Destination Lua table.
     */
    static void mapTreeToLua(const std::shared_ptr<quasar::named::NamedObject>& node, sol::table& table);
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_EXPRESSION_HPP
