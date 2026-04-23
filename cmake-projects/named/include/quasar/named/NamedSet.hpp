/**
 * @file NamedSet.hpp
 * @brief Template class for named set collections.
 */

#ifndef QUASAR_NAMED_NAMEDSET_HPP
#define QUASAR_NAMED_NAMEDSET_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <map>
#include <stdexcept>
#include <string>
#include <mutex>
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedSet
 * @brief A NamedObject that manages a set of unique elements identified by their intrinsic names.
 *
 * **Compliance**:
 * - Fulfills [FE-0110.2.1] Named Collections: NamedSet<T>.
 * - Fulfills [CS-0010.34] auto forbidden.
 *
 * @tparam T The type of elements.
 */
template <typename T>
class NamedSet : public NamedObject {
  static_assert(std::is_base_of<NamedObject, T>::value, "T must derive from NamedObject");
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~NamedSet() = default;

  /**
   * @brief Factory method.
   * @param name The name of the collection.
   * @param parent The optional parent.
   * @return A shared pointer to the new set.
   */
  static std::shared_ptr<NamedSet<T>> create(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    std::shared_ptr<NamedSet<T>> obj = std::make_shared<NamedSet<T>>(name);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Returns the type of the object.
   * @return "NamedSet"
   */
  std::string getType() const override { return "NamedSet"; }

  /**
   * @brief Inserts an element into the set.
   * If an element with the same name already exists, it is replaced.
   * @param item The item to insert.
   */
  void insert(std::shared_ptr<T> item) {
    if (!item) throw std::invalid_argument("Cannot insert null item in NamedSet");

    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");

    const std::string& key = item->getName();
    typename std::map<std::string, std::shared_ptr<T>>::iterator it = m_elements.find(key);
    if (it != m_elements.end()) {
        replaceChild(it->second, item);
    } else {
        addChild(item);
    }
  }

  /**
   * @brief Access element by its intrinsic name.
   * @param key The key.
   * @return The element.
   */
  std::shared_ptr<T> get(const std::string& key) const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");
    
    typename std::map<std::string, std::shared_ptr<T>>::const_iterator it = m_elements.find(key);
    if (it == m_elements.end()) {
       throw std::out_of_range("Key not found");
    }
    return it->second;
  }

  /**
   * @brief Check if an element with the given name exists.
   * @param key The key.
   * @return true if exists.
   */
  bool contains(const std::string& key) const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");
    return m_elements.find(key) != m_elements.end();
  }

  /**
   * @brief Removes the element by its intrinsic name.
   * @param key The key.
   * @return true if removed.
   */
  bool remove(const std::string& key) {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");

    typename std::map<std::string, std::shared_ptr<T>>::iterator it = m_elements.find(key);
    if (it != m_elements.end()) {
        removeChild(key);
        return true;
    }
    return false;
  }

  /**
   * @brief Current size of the set.
   * @return The size.
   */
  size_t size() const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");
    return m_elements.size();
  }

  /**
   * @brief Internal helper to add a child.
   * @param child The child.
   */
  void addChild(std::shared_ptr<NamedObject> child) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");
    
    NamedObject::addChild(child);

    std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(child);
    if (casted) {
        m_elements[child->getName()] = casted;
    }
  }

  /**
   * @brief Internal helper to remove a child by name.
   * @param name The name.
   */
  void removeChild(const std::string &name) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");

    std::string nameCopy = name;
    if (m_elements.count(nameCopy)) {
        this->NamedObject::removeChild(nameCopy);
        m_elements.erase(nameCopy);
    }
  }

  /**
   * @brief Internal helper to replace a child.
   * @param oldChild The old child.
   * @param newChild The new child.
   */
  void replaceChild(std::shared_ptr<NamedObject> oldChild, std::shared_ptr<NamedObject> newChild) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");
    
    NamedObject::replaceChild(oldChild, newChild);
    
    std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(newChild);
    if (casted) {
        m_elements.erase(oldChild->getName());
        m_elements[newChild->getName()] = casted;
    }
  }

  /**
   * @brief Iterator to the beginning.
   * @return Iterator.
   */
  typename std::map<std::string, std::shared_ptr<T>>::iterator begin() { return m_elements.begin(); }
  /**
   * @brief Iterator to the end.
   * @return Iterator.
   */
  typename std::map<std::string, std::shared_ptr<T>>::iterator end() { return m_elements.end(); }
  /**
   * @brief Const iterator to the beginning.
   * @return Iterator.
   */
  typename std::map<std::string, std::shared_ptr<T>>::const_iterator begin() const { return m_elements.begin(); }
  /**
   * @brief Const iterator to the end.
   * @return Iterator.
   */
  typename std::map<std::string, std::shared_ptr<T>>::const_iterator end() const { return m_elements.end(); }

  /**
   * @brief Clones the set.
   * @return Cloned object.
   */
  std::shared_ptr<NamedObject> clone([[maybe_unused]] CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    return NamedSet<T>::create(getName());
  }

  /**
   * @brief Constructor.
   * @param name The name.
   */
  explicit NamedSet(const std::string &name) : NamedObject(name) {}

protected:
  /** @brief Internal elements storage. */
  std::map<std::string, std::shared_ptr<T>> m_elements;
  /** @brief Mutex for protection. */
  mutable std::recursive_timed_mutex m_setMutex;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDSET_HPP

