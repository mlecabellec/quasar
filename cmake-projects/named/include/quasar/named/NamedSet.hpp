/**
 * @file NamedSet.hpp
 * @brief Template class for named set collections.
 */

#ifndef QUASAR_NAMED_NAMEDSET_HPP
#define QUASAR_NAMED_NAMEDSET_HPP

#include "quasar/named/NamedObject.hpp"
#include <map>
#include <stdexcept>
#include <string>
#include <mutex>

namespace quasar::named {

/**
 * @class NamedSet
 * @brief A NamedObject that manages a set of unique elements identified by their intrinsic names.
 *
 * **Compliance**:
 * - Fulfills [TSK-20260303-002.3] Named Collections: NamedSet<T>.
 *
 * @tparam T The type of elements.
 */
template <typename T>
class NamedSet : public NamedObject {
  static_assert(std::is_base_of<NamedObject, T>::value, "T must derive from NamedObject");
public:
  virtual ~NamedSet() = default;

  /**
   * @brief Factory method.
   */
  static std::shared_ptr<NamedSet<T>> create(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    auto obj = std::make_shared<NamedSet<T>>(name);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  std::string getType() const override { return "NamedSet"; }

  /**
   * @brief Inserts an element into the set.
   * If an element with the same name already exists, it is replaced.
   */
  void insert(std::shared_ptr<T> item) {
    if (!item) throw std::invalid_argument("Cannot insert null item in NamedSet");

    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");

    const std::string& key = item->getName();
    auto it = m_elements.find(key);
    if (it != m_elements.end()) {
        replaceChild(it->second, item);
    } else {
        addChild(item);
    }
  }

  /**
   * @brief Access element by its intrinsic name.
   */
  std::shared_ptr<T> get(const std::string& key) const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");
    
    auto it = m_elements.find(key);
    if (it == m_elements.end()) {
       throw std::out_of_range("Key not found");
    }
    return it->second;
  }

  /**
   * @brief Check if an element with the given name exists.
   */
  bool contains(const std::string& key) const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");
    return m_elements.find(key) != m_elements.end();
  }

  /**
   * @brief Removes the element by its intrinsic name.
   */
  bool remove(const std::string& key) {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");

    auto it = m_elements.find(key);
    if (it != m_elements.end()) {
        removeChild(key);
        return true;
    }
    return false;
  }

  size_t size() const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");
    return m_elements.size();
  }

  void addChild(std::shared_ptr<NamedObject> child) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");
    
    NamedObject::addChild(child);

    auto casted = std::dynamic_pointer_cast<T>(child);
    if (casted) {
        m_elements[child->getName()] = casted;
    }
  }

  void removeChild(const std::string &name) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");

    std::string nameCopy = name;
    if (m_elements.count(nameCopy)) {
        this->NamedObject::removeChild(nameCopy);
        m_elements.erase(nameCopy);
    }
  }

  void replaceChild(std::shared_ptr<NamedObject> oldChild, std::shared_ptr<NamedObject> newChild) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_setMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedSet lock");
    
    NamedObject::replaceChild(oldChild, newChild);
    
    auto casted = std::dynamic_pointer_cast<T>(newChild);
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
    return NamedSet<T>::create(getName());
  }

  // Constructor
  NamedSet(const std::string &name) : NamedObject(name) {}

private:
  std::map<std::string, std::shared_ptr<T>> m_elements;
  mutable std::recursive_timed_mutex m_setMutex;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDSET_HPP
