/**
 * @file NamedMap.hpp
 * @brief Template class for named key-value collections.
 */

#ifndef QUASAR_NAMED_NAMEDMAP_HPP
#define QUASAR_NAMED_NAMEDMAP_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <map>
#include <stdexcept>
#include <string>
#include <mutex>
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedMap
 * @brief A NamedObject that manages a dictionary of elements.
 *
 * **Compliance**:
 * - Fulfills [FE-0110.2.1] Named Collections: NamedMap<K,V>.
 * - Fulfills [CS-0010.34] auto forbidden.
 *
 * @tparam V The type of elements.
 */
template <typename V>
class NamedMap : public NamedObject {
  static_assert(std::is_base_of<NamedObject, V>::value, "V must derive from NamedObject");
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~NamedMap() = default;

  /**
   * @brief Factory method.
   * @param name The name of the collection.
   * @param parent The optional parent.
   * @return A shared pointer to the new map.
   */
  static std::shared_ptr<NamedMap<V>> create(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    std::shared_ptr<NamedMap<V>> obj = std::make_shared<NamedMap<V>>(name);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Returns the type of the object.
   * @return "NamedMap"
   */
  std::string getType() const override { return "NamedMap"; }

  /**
   * @brief Inserts or replaces an element.
   * Its name is updated to the given key.
   * @param key The key.
   * @param item The item.
   */
  void put(const std::string& key, std::shared_ptr<V> item) {
    if (!item) throw std::invalid_argument("Cannot insert null item in NamedMap");

    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");

    item->setName(key);
    
    // Check if key already exists, to replace it
    typename std::map<std::string, std::shared_ptr<V>>::iterator it = m_elements.find(key);
    if (it != m_elements.end()) {
        std::shared_ptr<V> oldItem = it->second;
        replaceChild(oldItem, item);
    } else {
        addChild(item);
    }
  }

  /**
   * @brief Access element by key.
   * @param key The key.
   * @return The element.
   */
  std::shared_ptr<V> get(const std::string& key) const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");
    
    typename std::map<std::string, std::shared_ptr<V>>::const_iterator it = m_elements.find(key);
    if (it == m_elements.end()) {
       throw std::out_of_range("Key not found");
    }
    return it->second;
  }

  /**
   * @brief Check if key exists.
   * @param key The key.
   * @return true if exists.
   */
  bool contains(const std::string& key) const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");
    return m_elements.find(key) != m_elements.end();
  }

  /**
   * @brief Removes the element by key.
   * @param key The key.
   * @return true if removed.
   */
  bool remove(const std::string& key) {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");

    typename std::map<std::string, std::shared_ptr<V>>::iterator it = m_elements.find(key);
    if (it != m_elements.end()) {
        removeChild(key);
        return true;
    }
    return false;
  }

  /**
   * @brief Current size of the map.
   * @return The size.
   */
  size_t size() const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");
    return m_elements.size();
  }

  /**
   * @brief Internal helper to add a child.
   * @param child The child.
   */
  void addChild(std::shared_ptr<NamedObject> child) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");
    
    NamedObject::addChild(child);

    std::shared_ptr<V> casted = std::dynamic_pointer_cast<V>(child);
    if (casted) {
        m_elements[child->getName()] = casted;
    }
  }

  /**
   * @brief Internal helper to remove a child by name.
   * @param name The name.
   */
  void removeChild(const std::string &name) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");

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
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");
    
    NamedObject::replaceChild(oldChild, newChild);
    
    std::shared_ptr<V> casted = std::dynamic_pointer_cast<V>(newChild);
    if (casted) {
        m_elements.erase(oldChild->getName());
        m_elements[newChild->getName()] = casted;
    }
  }

  /**
   * @brief Iterator to the beginning.
   * @return Iterator.
   */
  typename std::map<std::string, std::shared_ptr<V>>::iterator begin() { return m_elements.begin(); }
  /**
   * @brief Iterator to the end.
   * @return Iterator.
   */
  typename std::map<std::string, std::shared_ptr<V>>::iterator end() { return m_elements.end(); }
  /**
   * @brief Const iterator to the beginning.
   * @return Iterator.
   */
  typename std::map<std::string, std::shared_ptr<V>>::const_iterator begin() const { return m_elements.begin(); }
  /**
   * @brief Const iterator to the end.
   * @return Iterator.
   */
  typename std::map<std::string, std::shared_ptr<V>>::const_iterator end() const { return m_elements.end(); }

  /**
   * @brief Clones the map.
   * @return Cloned object.
   */
  std::shared_ptr<NamedObject> clone([[maybe_unused]] CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    return NamedMap<V>::create(getName());
  }

  /**
   * @brief Constructor.
   * @param name The name.
   */
  explicit NamedMap(const std::string &name) : NamedObject(name) {}

protected:
  /** @brief Internal elements storage. */
  std::map<std::string, std::shared_ptr<V>> m_elements;
  /** @brief Mutex for protection. */
  mutable std::recursive_timed_mutex m_mapMutex;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDMAP_HPP

