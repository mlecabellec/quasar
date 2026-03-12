#ifndef QUASAR_NAMED_TRAVERSAL_TRANSFORMCONTEXT_HPP
#define QUASAR_NAMED_TRAVERSAL_TRANSFORMCONTEXT_HPP

#include "quasar/named/NamedObject.hpp"
#include <memory>
#include <string>

namespace quasar::named::traversal {

/**
 * @class TransformContext
 * @brief Context for evaluating a node during transformation.
 */
class TransformContext {
public:
    TransformContext(std::shared_ptr<NamedObject> node, int depth, const std::string& path)
        : m_node(node), m_depth(depth), m_path(path) {}

    std::shared_ptr<NamedObject> getNode() const { return m_node; }
    int getDepth() const { return m_depth; }
    std::string getPath() const { return m_path; }

private:
    std::shared_ptr<NamedObject> m_node;
    int m_depth;
    std::string m_path;
};

} // namespace quasar::named::traversal

#endif // QUASAR_NAMED_TRAVERSAL_TRANSFORMCONTEXT_HPP
