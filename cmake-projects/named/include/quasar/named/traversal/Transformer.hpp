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
    Transformer() = default;

    /**
     * @brief Adds a rule to the engine. Higher priority rules beat lower priority ones.
     */
    void addRule(const TransformationRule& rule);

    void addRule(TransformPredicate p, TransformGenerator g, int priority = 0) {
        addRule(TransformationRule(std::move(p), std::move(g), priority));
    }

    /**
     * @brief Out-of-place transformation.
     */
    std::vector<std::shared_ptr<NamedObject>> transform(std::shared_ptr<NamedObject> root);

    /**
     * @brief In-place transformation. Modifies the tree directly.
     */
    void transformInPlace(std::shared_ptr<NamedObject> root);

private:
    std::vector<TransformationRule> m_rules;

    std::vector<std::shared_ptr<NamedObject>> transformRecursive(
        std::shared_ptr<NamedObject> node, int depth, const std::string& path);

    void transformInPlaceRecursive(std::shared_ptr<NamedObject> node, int depth, const std::string& path);
};

} // namespace quasar::named::traversal

#endif // QUASAR_NAMED_TRAVERSAL_TRANSFORMER_HPP
