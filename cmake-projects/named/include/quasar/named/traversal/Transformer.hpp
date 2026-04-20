#ifndef QUASAR_NAMED_TRAVERSAL_TRANSFORMER_HPP
#define QUASAR_NAMED_TRAVERSAL_TRANSFORMER_HPP

#include "quasar/named/traversal/TransformationRule.hpp"
#include <vector>
#include <memory>
#include <algorithm>

namespace quasar::named::traversal {

/**
 * @class Transformer
 * @brief An engine to apply rules to a NamedObject tree, XSLT-style.
 * 
 * @reference [TSK-20260311-001] XSLT-inspired tree transformation engine
 * @reference [FE-0150] Tree Transformation Engine
 */
class Transformer {
public:
    /** @brief Default constructor. */
    Transformer() = default;

    /**
     * @brief Adds a rule to the engine. Higher priority rules beat lower priority ones.
     * @param rule The rule to add.
     */
    void addRule(const TransformationRule& rule);

    /**
     * @brief Adds a rule using a predicate and generator.
     * @param p Matcher function.
     * @param g Generator function.
     * @param priority Rule priority.
     */
    void addRule(TransformPredicate p, TransformGenerator g, int priority = 0) {
        addRule(TransformationRule(std::move(p), std::move(g), priority));
    }

    /**
     * @brief Out-of-place transformation of a whole tree.
     * @param root The root of the tree to transform.
     * @return List of new root nodes.
     */
    std::vector<std::shared_ptr<NamedObject>> transform(std::shared_ptr<NamedObject> root);

    /**
     * @brief In-place transformation. Modifies the tree directly.
     * @param root The root node to start in-place transformation from.
     */
    void transformInPlace(std::shared_ptr<NamedObject> root);

    /**
     * @brief Recursively transforms a subtree. Exposing this allows generators to
     * manually trigger transformation on children.
     * 
     * Fulfills [TSK-20260311-001.1.3] manual recursive trigger.
     * 
     * @param node The node to transform.
     * @param depth Current recursion depth.
     * @param path Full logical path to the node.
     * @return List of transformed objects.
     */
    std::vector<std::shared_ptr<NamedObject>> transformSubtree(
        std::shared_ptr<NamedObject> node, int depth, const std::string& path);

private:
    /** @brief Collection of rules, sorted by priority. */
    std::vector<TransformationRule> m_rules;

    /**
     * @brief Internal helper for in-place transformation.
     */
    void transformInPlaceRecursive(std::shared_ptr<NamedObject> node, int depth, const std::string& path);
};

} // namespace quasar::named::traversal

#endif // QUASAR_NAMED_TRAVERSAL_TRANSFORMER_HPP
