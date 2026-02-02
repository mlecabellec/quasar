#ifndef QUASAR_NAMED_NAMEDOBJECT_HPP
#define QUASAR_NAMED_NAMEDOBJECT_HPP

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace quasar::named {

class NamedObject {
public:
  virtual ~NamedObject();

  /**
   * @brief Creates a new NamedObject.
   * @param name The name of the object.
   * @param parent The optional parent of the object.
   * @return A shared_ptr to the created object.
   * @throws std::runtime_error if name is invalid or not unique in parent.
   */
  static std::shared_ptr<NamedObject>
  create(const std::string &name,
         std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Sets the parent of the object.
   * @param parent The new parent. Can be null.
   * @throws std::runtime_error if name is not unique in new parent.
   */
  void setParent(std::shared_ptr<NamedObject> parent);

  /**
   * @brief Gets the name of the object.
   * @return The name.
   */
  std::string getName() const;

  /**
   * @brief Gets the parent of the object.
   * @return Shared pointer to parent, or null if no parent.
   */
  std::shared_ptr<NamedObject> getParent() const;

  /**
   * @brief Gets a list of children.
   * @return List of shared pointers to children.
   */
  std::list<std::shared_ptr<NamedObject>> getChildren() const;

  /**
   * @brief Gets the previous sibling in the parent's child list.
   * @return Shared pointer to previous sibling, or null.
   */
  std::shared_ptr<NamedObject> getPreviousSibling() const;

  /**
   * @brief Gets the next sibling in the parent's child list.
   * @return Shared pointer to next sibling, or null.
   */
  std::shared_ptr<NamedObject> getNextSibling() const;

  /**
   * @brief Gets the first child.
   * @return Shared pointer to first child, or null.
   */
  std::shared_ptr<NamedObject> getFirstChild() const;

  /**
   * @brief Gets the last child.
   * @return Shared pointer to last child, or null.
   */
  std::shared_ptr<NamedObject> getLastChild() const;

  /**
   * @brief Sets a related object (weak reference).
   * @param related The related object.
   */
  void setRelated(std::shared_ptr<NamedObject> related);

  /**
   * @brief Gets the related object.
   * @return Shared pointer to related object, or null.
   */
  std::shared_ptr<NamedObject> getRelated() const;

  // Comparison
  /**
   * @brief Checks equality based on name.
   * @param other The other object.
   * @return true if names are equal.
   */
  bool operator==(const NamedObject &other) const;

  /**
   * @brief Compares based on name (lexicographical).
   * @param other The other object.
   * @return true if this name < other name.
   */
  bool operator<(
      const NamedObject &other) const; // Lexicographical comparison by name

  /**
   * @brief Creates a clone of this object (without hierarchy).
   * @return Shared pointer to new cloned object.
   */
  virtual std::shared_ptr<NamedObject> clone() const;

  /**
   * @brief Gets a shared pointer to this object.
   * @return Shared pointer.
   */
  std::shared_ptr<NamedObject> getSelf() const;

protected:
  NamedObject(const std::string &name);

  // Internal helper to add a child (called by child's setParent or create)
  void addChild(std::shared_ptr<NamedObject> child);
  // Internal helper to remove a child
  void removeChild(const std::string &name);
  
  void setSelf(std::shared_ptr<NamedObject> self) {
      m_self = self;
  }

private:
  /** @brief The object name. */
  std::string m_name;
  /** @brief Weak pointer to parent. */
  std::weak_ptr<NamedObject> m_parent;
  /** @brief List of children. */
  std::list<std::shared_ptr<NamedObject>> m_children;
  /** @brief Weak pointer to related object. */
  std::weak_ptr<NamedObject> m_related;
  /** @brief Mutex for thread safety. */
  mutable std::recursive_mutex m_mutex;
  
  /** @brief Manual weak pointer to self. */
  std::weak_ptr<NamedObject> m_self;

  static bool isValidName(const std::string &name);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDOBJECT_HPP
