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
#include <atomic>

#include "quasar/named/CopyPolicy.hpp"
#include "quasar/named/IObserver.hpp"

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
 *
 * **Compliance**:
 * - Fulfills [FE-0020.1] Provide a "NamedObject" class.
 * - Fulfills [FE-0020.5] Operations on named objects shall be thread safe.
 * - Fulfills [FE-0030.8] All methods are thread safe.
 * - Fulfills [FE-0030.9] All methods are const correct.
 * - Fulfills [FE-0060.1.1] Provide name, parent, and child collection features.
 * - Fulfills [CS-0010.45] Documented with Doxygen.
 *
 * The class is thread-safe using a recursive mutex for all state-modifying
 * operations.
 */
class NamedObject {
public:
  /**
   * @brief Virtual destructor.
   * @compliance [FE-0020.1.3.2] Parent destruction releases children.
   */
  virtual ~NamedObject();

  /**
   * @brief Factory method to create a new NamedObject.
   *
   * This is the preferred way to create objects as it ensures proper
   * initialization of the internal weak reference to self.
   *
   * **Compliance**:
   * - Fulfills [FE-0020.6] static method "create" for object instantiation.
   * - Fulfills [FE-0020.6.1] Takes a name and an optional parent.
   * - Fulfills [FE-0020.6.2] Returns a std::shared_ptr.
   * - Fulfills [FE-0020.6.3] Throws std::runtime_error if the name is invalid.
   * - Fulfills [FE-0060.1.3] Validity of the name checked when created.
   *
   * @param name The name of the object. Must follow identifier rules
   * ([a-zA-Z_][a-zA-Z0-9_]*).
   * @param parent Optional parent to attach the new object to.
   * @return A shared_ptr to the newly created object.
   * @throws std::runtime_error if name is empty, invalid, or already exists in
   * the parent.
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
   * **Compliance**:
   * - Fulfills [FE-0020.1.2.1] Parent may be set later.
   * - Fulfills [FE-0020.1.2.2] Child added to parent's list.
   * - Fulfills [FE-0020.1.3.3] Child added to parent's children list.
   * - Fulfills [FE-0020.1.3.4] Child removed from parent's list when unset.
   *
   * @param parent The new parent object. Passing nullptr detaches the object.
   * @throws std::runtime_error if parent == this, if name is not unique in new
   * parent, or if a circular dependency is detected.
   */
  void setParent(std::shared_ptr<NamedObject> parent);

  /**
   * @brief Retrieves the object's name.
   *
   * @return The object name as a string.
   * @compliance [FE-0020.3.2] Getter for the name property.
   */
  std::string getName() const;

  /**
   * @brief Sets the object's name.
   *
   * @param name The new name for the object.
   * @throws std::runtime_error if name is empty, invalid, or already exists in the parent.
   */
  void setName(const std::string &name);

  /**
   * @brief Retrieves the parent object.
   *
   * @return Shared pointer to the parent, or nullptr if it's a root object.
   * @compliance [FE-0020.3.1] Getter for the parent property.
   */
  std::shared_ptr<NamedObject> getParent() const;

  /**
   * @brief Retrieves all child objects.
   *
   * @return A list containing shared pointers to all direct children.
   * @compliance [FE-0020.3.3] Getter for the children property.
   */
  std::list<std::shared_ptr<NamedObject>> getChildren() const;

  /**
   * @brief Retrieves a specific child by name.
   *
   * @param name The name of the child to find.
   * @return Shared pointer to the child, or nullptr if not found.
   */
  std::shared_ptr<NamedObject> getChild(const std::string &name) const;


  /**
   * @brief Finds the sibling immediately preceding this object in the parent's
   * child list.
   *
   * @return Shared pointer to the previous sibling, or nullptr if none.
   * @compliance [FE-0020.3.5] Getter for the previous sibling property.
   */
  std::shared_ptr<NamedObject> getPreviousSibling() const;

  /**
   * @brief Finds the sibling immediately following this object in the parent's
   * child list.
   *
   * @return Shared pointer to the next sibling, or nullptr if none.
   * @compliance [FE-0020.3.6] Getter for the next sibling property.
   */
  std::shared_ptr<NamedObject> getNextSibling() const;

  /**
   * @brief Retrieves the first child in the hierarchy.
   *
   * @return Shared pointer to the first child, or nullptr if no children.
   * @compliance [FE-0020.3.7] Getter for the first child property.
   */
  std::shared_ptr<NamedObject> getFirstChild() const;

  /**
   * @brief Retrieves the last child in the hierarchy.
   *
   * @return Shared pointer to the last child, or nullptr if no children.
   * @compliance [FE-0020.3.8] Getter for the last child property.
   */
  std::shared_ptr<NamedObject> getLastChild() const;

  /**
   * @brief Sets a weak relationship to another NamedObject.
   *
   * Related objects are typically non-hierarchical links.
   *
   * @param related The object to relate to.
   * @compliance [FE-0020.7] Support a "related" property.
   * @compliance [FE-0020.8] Relations implemented using weak pointers.
   */
  void setRelated(std::shared_ptr<NamedObject> related);

  /**
   * @brief Retrieves the related object.
   * @return Shared pointer to the related object, or nullptr if it has been
   * destroyed or was never set.
   */
  std::shared_ptr<NamedObject> getRelated() const;

  /**
   * @brief Equality operator.
   * Compares objects based on their names.
   *
   * @param other The object to compare with.
   * @return true if names match.
   * @compliance [FE-0020.2] Methods for comparison based on the name.
   */
  bool operator==(const NamedObject &other) const;

  /**
   * @brief Less-than operator for sorting.
   * Compares objects lexicographically based on their names.
   *
   * @param other The object to compare with.
   * @return true if this name is lexicographically smaller than the other.
   * @compliance [FE-0020.2.3] Comparison shall be lexicographical.
   */
  bool operator<(const NamedObject &other) const;

  /**
   * @brief Returns the type of the object as a string.
   *
   * @return The class name as a string. Default is "NamedObject".
   */
  virtual std::string getType() const;

  /**
   * @brief Checks if the object is of a specific type.
   * @tparam T The type to check for.
   * @return true if the object is of type T or derived from T.
   */
  template <typename T> bool is() const {
    return dynamic_cast<const T *>(this) != nullptr;
  }

  /**
   * @brief Casts the object to a specific type.
   * @tparam T The type to cast to.
   * @return Shared pointer to the object as type T, or nullptr if cast fails.
   */
  template <typename T> std::shared_ptr<T> as() {
    return std::dynamic_pointer_cast<T>(getSelf());
  }

  /**
   * @brief Replaces this object in the hierarchy with another object.
   *
   * This method updates the parent's reference and re-parents all children.
   *
   * @param replacement The object that will replace this one.
   * @throws std::runtime_error if timeout occurs or hierarchy becomes invalid.
   * @compliance [FE-0110.1.1] Tree Substitution.
   */
  void replaceInTree(std::shared_ptr<NamedObject> replacement);

  /**
   * @brief Creates a standalone copy of this object.
   *
   * This is a shallow clone that does not copy the hierarchy (parent/children).
   * Derived classes should override this to preserve their specific state.
   *
   * @return Shared pointer to a new object with the same name and data.
   */
  /**
   * @brief Broadcasts an event to all subscribed observers.
   * @param eventData Shared pointer containing the event payload.
   */
  void notifyObservers(std::shared_ptr<NamedObject> eventData);

  virtual std::shared_ptr<NamedObject> clone(CopyPolicy policy = CopyPolicy::DUPLICATE) const;

  /**
   * @brief Performs a deep copy of this object and its entire subtree.
   *
   * @return A deep clone of the tree starting from this object as the root.
   * @compliance [FE-0020.14] Utilities for copying parts of the tree.
   * @compliance [FE-0020.14.1] deep copy mechanism.
   */
  virtual std::shared_ptr<NamedObject> deepCopy(CopyPolicy policy = CopyPolicy::DUPLICATE, int maxDepth = 256) const;

  /**
   * @brief Internal recursive helper for deep copying a subtree.
   *
   * @param originalParent The parent in the original tree.
   * @param newParent The new parent in the cloned tree.
   * @param policy Memory policy.
   * @param maxDepth Maximum recursion depth to prevent stack overflow.
   * @return A deep clone of this object, appropriately re-parented and rebased
   * if needed.
   * @compliance [FE-0020.14.1] deep copy mechanism.
   */
  virtual std::shared_ptr<NamedObject>
  deepCopy(std::shared_ptr<NamedObject> originalParent,
           std::shared_ptr<NamedObject> newParent, CopyPolicy policy = CopyPolicy::DUPLICATE, int maxDepth = 256) const;

  /**
   * @brief Returns a shared pointer to this instance.
   * @return Shared pointer to self.
   * @exposed
   */
  [[nodiscard]] std::shared_ptr<NamedObject> getSelf() const;

  /**
   * @brief Returns the current structural version of this node and its children.
   * 
   * The version increments whenever a structural change (add/remove/rename) occurs 
   * in this node or any of its descendants.
   * 
   * @return The current version number.
   * @feature TSK-20260311-008.6 REST API Cache Consistency
   * @exposed
   */
  [[nodiscard]] uint64_t getTreeVersion() const;

  /**
   * @brief Helper to propagate structural version increments up the tree.
   * 
   * Implementation must avoid infinite recursion via iteration limits.
   * @feature TSK-20260311-008.6 Structural Tracking
   */
  void incrementTreeVersion();

  /**
   * @brief Subscribes an observer to this object's events.
   * @param observer Weak pointer to the observer.
   * @exposed
   */
  void subscribe(std::weak_ptr<IObserver> observer);

  /**
   * @brief Unsubscribes an observer.
   * @param observer Weak pointer to the observer to remove.
   * @exposed
   */
  void unsubscribe(std::weak_ptr<IObserver> observer);

protected:
  /** @brief Recursion depth limit for tree updates. */
  static constexpr int MAX_TREE_UPDATE_DEPTH = 1024;

  /** @brief Depth-limited internal version incrementer. [CS-0010.37] */
  void incrementTreeVersionInternal(int depth);

  /**
   * @brief Protected constructor to enforce use of create() factory.
   * @param name The name of the object.
   * @compliance [FE-0020.1.1] "name" property initialized.
   */
  NamedObject(const std::string &name);

  /**
   * @brief Internal helper to add a child.
   * Used by setParent and create.
   * @param child The child to add.
   * @compliance [FE-0020.1.3.1] strong reference to children.
   */
  virtual void addChild(std::shared_ptr<NamedObject> child);

  /**
   * @brief Internal helper to replace a child.
   * @param oldChild The child to be replaced.
   * @param newChild The replacement child.
   */
  virtual void replaceChild(std::shared_ptr<NamedObject> oldChild,
                            std::shared_ptr<NamedObject> newChild);

  /**
   * @brief Internal helper to remove a child by name.
   * Used by setParent.
   * @param name Name of the child to remove.
   * @compliance [FE-0020.1.3.4] removed from parent list.
   */
  virtual void removeChild(const std::string &name);

  /**
   * @brief Sets the internal weak pointer to self.
   * @param self Shared pointer to self.
   */
  void setSelf(std::shared_ptr<NamedObject> self) { m_self = self; }

protected:
  /** @brief The object name. Unique within parent scope. */
  std::string m_name;
  /** @brief Weak pointer to parent to avoid circular shared_ptr dependencies.
   */
  std::weak_ptr<NamedObject> m_parent;
  /** @brief List of direct children, maintaining ownership. */
  std::list<std::shared_ptr<NamedObject>> m_children;
  /** @brief Weak pointer to a related object. */
  std::weak_ptr<NamedObject> m_related;
  /** @brief Mutex for ensuring thread-safe access to internal state. */
  mutable std::recursive_timed_mutex m_mutex;

  /** @brief Internal weak pointer to self, used for retrieving a shared_ptr
   * from this. */
  std::weak_ptr<NamedObject> m_self;

  /** 
   * @brief Counter tracking structural modifications within this node and its children. 
   * @feature TSK-20260311-008.6 REST API Cache Consistency
   */
  std::atomic<uint64_t> m_treeVersion{0};

  /** @brief Mutex for observer list protection. */
  mutable std::recursive_timed_mutex m_observerMutex;
  /** @brief List of weak pointers to observers. */
  std::vector<std::weak_ptr<IObserver>> m_observers;

  /**
   * @brief Validates if a name follows the allowed format.
   * @param name The name to validate.
   * @return true if valid.
   * @compliance [FE-0020.1.1.3] Name shall match regex.
   */
  static bool isValidName(const std::string &name);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDOBJECT_HPP

