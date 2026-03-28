#include "quasar/named/traversal/Transformer.hpp"

namespace quasar::named::traversal {

void Transformer::addRule(const TransformationRule& rule) {
    m_rules.push_back(rule);
    // Sort descending by priority so that highest priority is first
    std::stable_sort(m_rules.begin(), m_rules.end(), [](const TransformationRule& a, const TransformationRule& b) {
        return a.getPriority() > b.getPriority();
    });
}

std::vector<std::shared_ptr<NamedObject>> Transformer::transform(std::shared_ptr<NamedObject> root) {
    if (!root) return {};
    return transformRecursive(root, 0, root->getName());
}

std::vector<std::shared_ptr<NamedObject>> Transformer::transformRecursive(
    std::shared_ptr<NamedObject> node, int depth, const std::string& path) {
    
    TransformContext ctx(node, depth, path);

    for (const TransformationRule& rule : m_rules) {
        if (rule.matches(ctx)) {
            // Apply rule. It decides if/how to recurse.
            return rule.apply(ctx);
        }
    }

    // Default behavior if no rules match: deep copy the node and recursively apply to children
    std::shared_ptr<NamedObject> cloned = node->clone(CopyPolicy::DUPLICATE);
    for (std::shared_ptr<NamedObject>& child : node->getChildren()) {
        std::string childPath = path + "/" + child->getName();
        std::vector<std::shared_ptr<NamedObject>> transformedChildren = transformRecursive(child, depth + 1, childPath);
        for (std::shared_ptr<NamedObject>& tc : transformedChildren) {
            tc->setParent(cloned);
        }
    }
    return {cloned};
}

void Transformer::transformInPlace(std::shared_ptr<NamedObject> root) {
    if (!root) return;
    transformInPlaceRecursive(root, 0, root->getName());
}

void Transformer::transformInPlaceRecursive(std::shared_ptr<NamedObject> node, int depth, const std::string& path) {
    // In-place transformation is riskier.
    // We get a snapshot of children to avoid iterator invalidation.
    std::list<std::shared_ptr<NamedObject>> children = node->getChildren();
    
    for (std::shared_ptr<NamedObject>& child : children) {
        std::string childPath = path + "/" + child->getName();
        TransformContext ctx(child, depth + 1, childPath);
        
        bool matched = false;
        for (const TransformationRule& rule : m_rules) {
            if (rule.matches(ctx)) {
                matched = true;
                std::vector<std::shared_ptr<NamedObject>> generated = rule.apply(ctx);
                
                // Replace child with generated sequence
                // First remove the old child
                child->setParent(nullptr); // detaches from node
                
                // Then add the new ones
                for (std::shared_ptr<NamedObject>& g : generated) {
                    g->setParent(node);
                }
                break;
            }
        }
        
        if (!matched) {
            // No rule matched, recurse into this child
            transformInPlaceRecursive(child, depth + 1, childPath);
        }
    }
}

} // namespace quasar::named::traversal
