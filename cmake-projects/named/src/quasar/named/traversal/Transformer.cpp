#include "quasar/named/traversal/Transformer.hpp"
#include <stdexcept>

namespace quasar::named::traversal {

/** @brief Hard limit on recursion depth to comply with CS-0010.38. */
constexpr int QUASAR_MAX_TRANSFORM_DEPTH = 256;

void Transformer::addRule(const TransformationRule& rule) {
    m_rules.push_back(rule);
    // Sort descending by priority so that highest priority is first.
    std::stable_sort(m_rules.begin(), m_rules.end(), [](const TransformationRule& a, const TransformationRule& b) {
        return a.getPriority() > b.getPriority();
    });
}

std::vector<std::shared_ptr<NamedObject>> Transformer::transform(std::shared_ptr<NamedObject> root) {
    if (!root) {
        return {};
    }
    return transformSubtree(root, 0, root->getName());
}

std::vector<std::shared_ptr<NamedObject>> Transformer::transformSubtree(
    std::shared_ptr<NamedObject> node, int depth, const std::string& path) {
    
    // Validate recursion depth according to CS-0010.38.
    if (depth > QUASAR_MAX_TRANSFORM_DEPTH) {
        throw std::runtime_error("Maximum transformation depth exceeded");
    }

    TransformContext ctx(node, depth, path);

    // Rule priority evaluation logic.
    for (const TransformationRule& rule : m_rules) {
        if (rule.matches(ctx)) {
            // Rule found. Generator now handles subtree recursion if it chooses.
            return rule.apply(ctx, *this);
        }
    }

    // Default behavior if no rules match: deep copy the node and recursively apply to children.
    std::shared_ptr<NamedObject> cloned = node->clone(CopyPolicy::DUPLICATE);
    std::list<std::shared_ptr<NamedObject>> children = node->getChildren();

    for (const std::shared_ptr<NamedObject>& child : children) {
        std::string childPath = path + "/" + child->getName();
        std::vector<std::shared_ptr<NamedObject>> transformedChildren = transformSubtree(child, depth + 1, childPath);
        for (const std::shared_ptr<NamedObject>& tc : transformedChildren) {
            tc->setParent(cloned);
        }
    }
    return {cloned};
}

void Transformer::transformInPlace(std::shared_ptr<NamedObject> root) {
    if (!root) {
        return;
    }
    transformInPlaceRecursive(root, 0, root->getName());
}

void Transformer::transformInPlaceRecursive(std::shared_ptr<NamedObject> node, int depth, const std::string& path) {
    if (depth > QUASAR_MAX_TRANSFORM_DEPTH) {
        throw std::runtime_error("Maximum transformation depth exceeded in-place");
    }

    // Snapshot children to prevent iterator invalidation during mutation.
    std::list<std::shared_ptr<NamedObject>> children = node->getChildren();
    
    for (const std::shared_ptr<NamedObject>& child : children) {
        std::string childPath = path + "/" + child->getName();
        TransformContext ctx(child, depth + 1, childPath);
        
        bool matched = false;
        for (const TransformationRule& rule : m_rules) {
            if (rule.matches(ctx)) {
                matched = true;
                // Generator in in-place mode might return new nodes to insert.
                std::vector<std::shared_ptr<NamedObject>> generated = rule.apply(ctx, *this);
                
                // Remove the old child and attach generated replacements.
                child->setParent(nullptr); 
                for (const std::shared_ptr<NamedObject>& g : generated) {
                    g->setParent(node);
                }
                break;
            }
        }
        
        if (!matched) {
            // Recurse into branch if no rule matched the child.
            transformInPlaceRecursive(child, depth + 1, childPath);
        }
    }
}

} // namespace quasar::named::traversal
