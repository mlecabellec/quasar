#ifndef QUASAR_NAMED_TRAVERSAL_TRANSFORMATIONRULE_HPP
#define QUASAR_NAMED_TRAVERSAL_TRANSFORMATIONRULE_HPP

#include "quasar/named/traversal/TransformContext.hpp"
#include "quasar/named/NamedObject.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace quasar::named::traversal {

/**
 * @brief Predicate type for matching a rule against a context.
 */
using TransformPredicate = std::function<bool(const TransformContext&)>;

/**
 * @brief Generator type for producing new nodes from a context.
 */
using TransformGenerator = std::function<std::vector<std::shared_ptr<NamedObject>>(const TransformContext&)>;

/**
 * @class TransformationRule
 * @brief Defines a rule comprising a predicate (when to apply) and a generator (how to apply).
 */
class TransformationRule {
public:
    TransformationRule(TransformPredicate predicate, TransformGenerator generator, int priority = 0)
        : m_predicate(std::move(predicate)), m_generator(std::move(generator)), m_priority(priority) {}

    bool matches(const TransformContext& context) const {
        return m_predicate && m_predicate(context);
    }

    std::vector<std::shared_ptr<NamedObject>> apply(const TransformContext& context) const {
        if (!m_generator) return {};
        return m_generator(context);
    }

    int getPriority() const { return m_priority; }

private:
    TransformPredicate m_predicate;
    TransformGenerator m_generator;
    int m_priority;
};

} // namespace quasar::named::traversal

#endif // QUASAR_NAMED_TRAVERSAL_TRANSFORMATIONRULE_HPP
