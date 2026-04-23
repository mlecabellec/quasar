#ifndef QUASAR_NAMED_TRAVERSAL_TRANSFORMATIONRULE_HPP
#define QUASAR_NAMED_TRAVERSAL_TRANSFORMATIONRULE_HPP

#include "quasar/named/traversal/TransformContext.hpp"
#include "quasar/named/NamedObject.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace quasar::named::traversal {

class Transformer;

/**
 * @brief Predicate type for matching a rule against a context.
 */
using TransformPredicate = std::function<bool(const TransformContext&)>;

/**
 * @brief Generator type for producing new nodes from a context.
 * It takes the context and a reference to the transformer to allow manual recursion.
 */
using TransformGenerator = std::function<std::vector<std::shared_ptr<NamedObject>>(const TransformContext&, Transformer&)>;

/**
 * @class TransformationRule
 * @brief Defines a rule comprising a predicate (when to apply) and a generator (how to apply).
 * 
 * Rules are the primary building blocks of the Transformation Engine. Each rule
 * encapsulates the logic for identifying a specific type of node and how to
 * morph it into zero or more output nodes.
 * 
 * @feature [TSK-20260311-001.1.1] Rules and Matchers.
 */
class TransformationRule {
public:
    /**
     * @brief Constructor for TransformationRule.
     * @param predicate Condition for rule application.
     * @param generator Function to produce transformed nodes.
     * @param priority Execution priority (higher is better).
     */
    TransformationRule(TransformPredicate predicate, TransformGenerator generator, int priority = 0)
        : m_predicate(std::move(predicate)), m_generator(std::move(generator)), m_priority(priority) {}

    /**
     * @brief Checks if the rule matches the given context.
     * @param context The current transformation context.
     * @return true if the predicate evaluates to true.
     */
    bool matches(const TransformContext& context) const {
        return m_predicate && m_predicate(context);
    }

    /**
     * @brief Applies the rule to the context.
     * @param context The current transformation context.
     * @param transformer Reference to the active transformer engine.
     * @return List of generated objects.
     */
    std::vector<std::shared_ptr<NamedObject>> apply(const TransformContext& context, Transformer& transformer) const {
        if (!m_generator) return {};
        return m_generator(context, transformer);
    }

    /** @brief Returns rule priority. */
    int getPriority() const { return m_priority; }

protected:
    /** @brief Matcher function. */
    TransformPredicate m_predicate;
    /** @brief Generator function. */
    TransformGenerator m_generator;
    /** @brief Priority value. */
    int m_priority;
};

} // namespace quasar::named::traversal

#endif // QUASAR_NAMED_TRAVERSAL_TRANSFORMATIONRULE_HPP
