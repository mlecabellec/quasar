/**
 * @file NamedObject.hpp
 * @brief Core class for named hierarchical objects.
 */

#ifndef QUASAR_NAMED_NAMEDOBJECT_HPP
#define QUASAR_NAMED_NAMEDOBJECT_HPP

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

/**
 * @namespace quasar::named
 * @brief Namespace for named hierarchical object management.
 */
namespace quasar::named {

/**
 * @class NamedObject
 * @brief Base class for objects with a name and hierarchical relationships.
 * 
 * NamedObject provides a foundation for creating a tree of named objects.
 * Each object has a unique name within its parent's scope.
 * The class is thread-safe using a recursive mutex for all state-modifying operations.
 */
class NamedObject {
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~NamedObject();

  /**
   * @brief Factory method to create a new NamedObject.
   * 
   * This is the preferred way to create objects as it ensures proper initialization
   * of the internal weak reference to self.
   * 
   * @param name The name of the object. Must follow identifier rules ([a-zA-Z_][a-zA-Z0-9_]*).
   * @param parent Optional parent to attach the new object to.
   * @return A shared_ptr to the newly created object.
   * @throws std::runtime_error if name is empty, invalid, or already exists in the parent.
   */
  static std::shared_ptr<NamedObject>
  create(const std::string &name,
         std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Sets or changes the parent of this object.
   * 
   * This method handles moving the object from its old parent to the new one,
   * ensuring consistency and thread safety.
   * 
   * @param parent The new parent object. Passing nullptr detaches the object.
   * @throws std::runtime_error if parent == this, if name is not unique in new parent, 
   *         or if a circular dependency is detected.
   */
  void setParent(std::shared_ptr<NamedObject> parent);

  /**
   * @brief Retrieves the object's name.
   * @return The object name as a string.
   */
  std::string getName() const;

  /**
   * @brief Retrieves the parent object.
   * @return Shared pointer to the parent, or nullptr if it's a root object.
   */
  std::shared_ptr<NamedObject> getParent() const;

  /**
   * @brief Retrieves all child objects.
   * @return A list containing shared pointers to all direct children.
   */
  std::list<std::shared_ptr<NamedObject>> getChildren() const;

  /**
   * @brief Finds the sibling immediately preceding this object in the parent's child list.
   * @return Shared pointer to the previous sibling, or nullptr if none.
   */
  std::shared_ptr<NamedObject> getPreviousSibling() const;

  /**
   * @brief Finds the sibling immediately following this object in the parent's child list.
   * @return Shared pointer to the next sibling, or nullptr if none.
   */
  std::shared_ptr<NamedObject> getNextSibling() const;

  /**
   * @brief Retrieves the first child in the hierarchy.
   * @return Shared pointer to the first child, or nullptr if no children.
   */
  std::shared_ptr<NamedObject> getFirstChild() const;

  /**
   * @brief Retrieves the last child in the hierarchy.
   * @return Shared pointer to the last child, or nullptr if no children.
   */
  std::shared_ptr<NamedObject> getLastChild() const;

  /**
   * @brief Sets a weak relationship to another NamedObject.
   * 
   * Related objects are typically non-hierarchical links.
   * 
   * @param related The object to relate to.
   */
  void setRelated(std::shared_ptr<NamedObject> related);

  /**
   * @brief Retrieves the related object.
   * @return Shared pointer to the related object, or nullptr if it has been destroyed or was never set.
   */
  std::shared_ptr<NamedObject> getRelated() const;

  // Comparison
  /**
   * @brief Equality operator.
   * Compares objects based on their names.
   * @param other The object to compare with.
   * @return true if names match.
   */
  bool operator==(const NamedObject &other) const;

  /**
   * @brief Less-than operator for sorting.
   * Compares objects lexicographically based on their names.
   * @param other The object to compare with.
   * @return true if this name is lexicographically smaller than the other.
   */
  bool operator<(
      const NamedObject &other) const;

  /**
   * @brief Creates a standalone copy of this object.
   * 
   * This is a shallow clone that does not copy the hierarchy (parent/children).
   * Derived classes should override this to preserve their specific state.
   * 
   * @return Shared pointer to a new object with the same name and data.
   */
  virtual std::shared_ptr<NamedObject> clone() const;

  /**
   * @brief Returns a shared pointer to this instance.
   * @return Shared pointer to self.
   */
  std::shared_ptr<NamedObject> getSelf() const;

protected:
  /**
   * @brief Protected constructor to enforce use of create() factory.
   * @param name The name of the object.
   */
  NamedObject(const std::string &name);

  /**
   * @brief Internal helper to add a child.
   * Used by setParent and create.
   * @param child The child to add.
   */
  void addChild(std::shared_ptr<NamedObject> child);

  /**
   * @brief Internal helper to remove a child by name.
   * Used by setParent.
   * @param name Name of the child to remove.
   */
  void removeChild(const std::string &name);
  
  /**
   * @brief Sets the internal weak pointer to self.
   * @param self Shared pointer to self.
   */
  void setSelf(std::shared_ptr<NamedObject> self) {
      m_self = self;
  }

private:
  /** @brief The object name. Unique within parent scope. */
  std::string m_name;
  /** @brief Weak pointer to parent to avoid circular shared_ptr dependencies. */
  std::weak_ptr<NamedObject> m_parent;
  /** @brief List of direct children, maintaining ownership. */
  std::list<std::shared_ptr<NamedObject>> m_children;
  /** @brief Weak pointer to a related object. */
  std::weak_ptr<NamedObject> m_related;
  /** @brief Mutex for ensuring thread-safe access to internal state. */
  mutable std::recursive_mutex m_mutex;
  
  /** @brief Internal weak pointer to self, used for retrieving a shared_ptr from this. */
  std::weak_ptr<NamedObject> m_self;

  /**
   * @brief Validates if a name follows the allowed format.
   * @param name The name to validate.
   * @return true if valid.
   */
  static bool isValidName(const std::string &name);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDOBJECT_HPP
