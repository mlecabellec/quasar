/**
 * @file NamedMap.hpp
 * @brief Template class for named key-value collections.
 */

#ifndef QUASAR_NAMED_NAMEDMAP_HPP
#define QUASAR_NAMED_NAMEDMAP_HPP

#include "quasar/named/NamedObject.hpp"
#include <map>
#include <stdexcept>
#include <string>
#include <mutex>

namespace quasar::named {

/**
 * @class NamedMap
 * @brief A NamedObject that manages a dictionary of elements.
 *
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260303-002.3] Named Collections: NamedMap<K,V>.
 *
 * @tparam V The type of elements.
 */
template <typename V>
class NamedMap : public NamedObject {
  static_assert(std::is_base_of<NamedObject, V>::value, "V must derive from NamedObject");
public:
  virtual ~NamedMap() = default;

  /**
   * @brief Factory method.
   */
  static std::shared_ptr<NamedMap<V>> create(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    auto obj = std::make_shared<NamedMap<V>>(name);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  std::string getType() const override { return "NamedMap"; }

  /**
   * @brief Inserts or replaces an element.
   * Its name is updated to the given key.
   */
  void put(const std::string& key, std::shared_ptr<V> item) {
    if (!item) throw std::invalid_argument("Cannot insert null item in NamedMap");

    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");

    item->setName(key);
    
    // Check if key already exists, to replace it
    auto it = m_elements.find(key);
    if (it != m_elements.end()) {
        replaceChild(it->second, item);
        it->second = item;
    } else {
        addChild(item);
        m_elements[key] = item;
    }
  }

  /**
   * @brief Access element by key.
   */
  std::shared_ptr<V> get(const std::string& key) const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");
    
    auto it = m_elements.find(key);
    if (it == m_elements.end()) {
       throw std::out_of_range("Key not found");
    }
    return it->second;
  }

  /**
   * @brief Check if key exists.
   */
  bool contains(const std::string& key) const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");
    return m_elements.find(key) != m_elements.end();
  }

  /**
   * @brief Removes the element by key.
   */
  bool remove(const std::string& key) {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");

    auto it = m_elements.find(key);
    if (it != m_elements.end()) {
        removeChild(key);
        return true;
    }
    return false;
  }

  size_t size() const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");
    return m_elements.size();
  }

  void addChild(std::shared_ptr<NamedObject> child) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");
    
    NamedObject::addChild(child);

    auto casted = std::dynamic_pointer_cast<V>(child);
    if (casted) {
        m_elements[child->getName()] = casted;
    }
  }

  void removeChild(const std::string &name) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");

    std::string nameCopy = name;
    if (m_elements.count(nameCopy)) {
        this->NamedObject::removeChild(nameCopy);
        m_elements.erase(nameCopy);
    }
  }

  void replaceChild(std::shared_ptr<NamedObject> oldChild, std::shared_ptr<NamedObject> newChild) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mapMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedMap lock");
    
    NamedObject::replaceChild(oldChild, newChild);
    
    auto casted = std::dynamic_pointer_cast<V>(newChild);
    if (casted) {
        m_elements.erase(oldChild->getName());
        m_elements[newChild->getName()] = casted;
    }
  }

  auto begin() { return m_elements.begin(); }
  auto end() { return m_elements.end(); }
  auto begin() const { return m_elements.begin(); }
  auto end() const { return m_elements.end(); }

  std::shared_ptr<NamedObject> clone() const override {
    return NamedMap<V>::create(getName());
  }

  // Constructor
  NamedMap(const std::string &name) : NamedObject(name) {}

private:
  std::map<std::string, std::shared_ptr<V>> m_elements;
  mutable std::recursive_timed_mutex m_mapMutex;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDMAP_HPP
