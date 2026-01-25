#ifndef QUASAR_NAMED_NAMEDOBJECT_H
#define QUASAR_NAMED_NAMEDOBJECT_H

#include <string>
#include <memory>
#include <list>
#include <mutex>
#include <vector>

namespace quasar::named {

class NamedObject : public std::enable_shared_from_this<NamedObject> {
public:
    virtual ~NamedObject();

    /**
     * @brief Creates a new NamedObject.
     * @param name The name of the object.
     * @param parent The optional parent of the object.
     * @return A shared_ptr to the created object.
     * @throws std::runtime_error if name is invalid or not unique in parent.
     */
    static std::shared_ptr<NamedObject> create(const std::string& name, std::shared_ptr<NamedObject> parent = nullptr);

    /**
     * @brief Sets the parent of the object.
     * @param parent The new parent. Can be null.
     * @throws std::runtime_error if name is not unique in new parent.
     */
    void setParent(std::shared_ptr<NamedObject> parent);

    std::string getName() const;
    std::shared_ptr<NamedObject> getParent() const;
    std::list<std::shared_ptr<NamedObject>> getChildren() const;

    std::shared_ptr<NamedObject> getPreviousSibling() const;
    std::shared_ptr<NamedObject> getNextSibling() const;
    std::shared_ptr<NamedObject> getFirstChild() const;
    std::shared_ptr<NamedObject> getLastChild() const;

    void setRelated(std::shared_ptr<NamedObject> related);
    std::shared_ptr<NamedObject> getRelated() const;

    // Comparison
    bool operator==(const NamedObject& other) const;
    bool operator<(const NamedObject& other) const; // Lexicographical comparison by name

    virtual std::shared_ptr<NamedObject> clone() const;

protected:
    NamedObject(const std::string& name);

    // Internal helper to add a child (called by child's setParent or create)
    void addChild(std::shared_ptr<NamedObject> child);
    // Internal helper to remove a child
    void removeChild(const std::string& name);

private:
    std::string m_name;
    std::weak_ptr<NamedObject> m_parent;
    std::list<std::shared_ptr<NamedObject>> m_children;
    std::weak_ptr<NamedObject> m_related;
    mutable std::recursive_mutex m_mutex;

    static bool isValidName(const std::string& name);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDOBJECT_H
