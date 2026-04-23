#ifndef QUASAR_NAMED_TRAVERSAL_TRANSFORMCONTEXT_HPP
#define QUASAR_NAMED_TRAVERSAL_TRANSFORMCONTEXT_HPP

#include "quasar/named/NamedObject.hpp"
#include <memory>
#include <string>

namespace quasar::named::traversal {

/**
 * @class TransformContext
 * @brief Context for evaluating a node during transformation.
 * 
 * Provides metadata about the node being currently processed, including its
 * recursion depth and full path. This context is used by Predicates and
 * Generators to make decisions about the transformation logic.
 * 
 * @feature [TSK-20260311-001.1.3] Transformation Context.
 */
class TransformContext {
public:
    /**
     * @brief Constructor for TransformContext.
     * @param node The NamedObject being processed.
     * @param depth Current depth in the tree (root = 0).
     * @param path Full logical path string.
     */
    TransformContext(std::shared_ptr<NamedObject> node, int depth, const std::string& path)
        : m_node(node), m_depth(depth), m_path(path) {}

    /** @brief Returns the node being processed. */
    std::shared_ptr<NamedObject> getNode() const { return m_node; }
    /** @brief Returns the current recursion depth. */
    int getDepth() const { return m_depth; }
    /** @brief Returns the full logical path to the node. */
    std::string getPath() const { return m_path; }

protected:
    std::shared_ptr<NamedObject> m_node;
    int m_depth;
    std::string m_path;
};

} // namespace quasar::named::traversal

#endif // QUASAR_NAMED_TRAVERSAL_TRANSFORMCONTEXT_HPP
