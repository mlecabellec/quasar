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
 * The Transformer allows for declarative, rule-based morphing of hierarchical
 * structures. It supports both out-of-place (functional) and in-place (mutating)
 * execution modes.
 * 
 * **Compliance**:
 * - Fulfills [FE-0150] Tree Transformation Engine.
 * - Fulfills [CS-0010.45] Doxygen mandatory.
 * 
 * @feature [TSK-20260311-001] XSLT-inspired tree transformation engine.
 */
class Transformer {
public:
    /**
     * @brief Default constructor.
     * @contribution [TSK-20260311-001] Initial implementation.
     */
    Transformer() = default;

    /**
     * @brief Adds a rule to the engine. Higher priority rules beat lower priority ones.
     * 
     * Rules are automatically sorted by priority upon addition to ensure
     * deterministic evaluation.
     * 
     * @param rule The rule to add.
     * @feature [TSK-20260311-001.1.1] Rules and Matchers.
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
     * 
     * Evaluates rules against the source tree and produces a new, physically
     * independent or shared tree depending on the rule implementation.
     * 
     * @param root The root of the tree to transform.
     * @return List of new root nodes.
     * @feature [TSK-20260311-001.2] Out-of-Place (Functional) Mode.
     */
    std::vector<std::shared_ptr<NamedObject>> transform(std::shared_ptr<NamedObject> root);

    /**
     * @brief In-place transformation. Modifies the tree directly.
     * 
     * This mode is destructive to the original hierarchy but maintains
     * thread-safety via NamedObject's internal mutexes.
     * 
     * @param root The root node to start in-place transformation from.
     * @feature [TSK-20260311-001.3] In-Place (Mutating) Mode.
     */
    void transformInPlace(std::shared_ptr<NamedObject> root);

    /**
     * @brief Recursively transforms a subtree. Exposing this allows generators to
     * manually trigger transformation on children.
     * 
     * Analogous to <xsl:apply-templates/> in XSLT.
     * 
     * @param node The node to transform.
     * @param depth Current recursion depth.
     * @param path Full logical path to the node.
     * @return List of transformed objects.
     * @feature [TSK-20260311-001.1.3] Transformation Context & Manual Recursion.
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
