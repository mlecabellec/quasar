/**
 * @file NamedArray.hpp
 * @brief Template class for named array collections.
 */

#ifndef QUASAR_NAMED_NAMEDARRAY_HPP
#define QUASAR_NAMED_NAMEDARRAY_HPP

#include "quasar/named/NamedObject.hpp"
#include <vector>
#include <stdexcept>
#include <string>
#include <mutex>

namespace quasar::named {

/**
 * @class NamedArray
 * @brief A NamedObject that manages a collection of elements.
 *
 * Managing elements as NamedObject children allows iterating over them
 * generically, while elements are named according to their index ("0", "1", ...).
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260303-002.3] Named Collections: NamedArray<T>.
 *
 * @tparam T The type of elements, must derive from NamedObject.
 */
template <typename T>
class NamedArray : public NamedObject {
  static_assert(std::is_base_of<NamedObject, T>::value, "T must derive from NamedObject");
public:
  virtual ~NamedArray() = default;

  /**
   * @brief Factory method.
   */
  static std::shared_ptr<NamedArray<T>> create(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    auto obj = std::make_shared<NamedArray<T>>(name);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  std::string getType() const override { return "NamedArray"; }

  /**
   * @brief Access element by index.
   */
  std::shared_ptr<T> get(size_t index) const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");
    if (index >= m_elements.size()) {
       throw std::out_of_range("Index out of bounds");
    }
    return m_elements[index];
  }

  /**
   * @brief Appends an element to the array.
   * Its name is changed to the index number.
   */
  void push_back(std::shared_ptr<T> item) {
    if (!item) throw std::invalid_argument("Cannot insert null item in NamedArray");

    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");

    size_t newIndex = m_elements.size();
    std::string newName = "_" + std::to_string(newIndex);
    
    item->setName(newName);
    
    // Add to NamedObject child tree
    addChild(item);
  }

  /**
   * @brief Removes the element at the specified index and shifts subsequent elements.
   */
  void removeAt(size_t index) {
    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");

    if (index >= m_elements.size()) {
       throw std::out_of_range("Index out of bounds");
    }

    auto item = m_elements[index];
    
    // Remove from NamedObject child tree
    removeChild(item->getName());

    // Rename subsequent elements to keep index names contiguous
    for (size_t i = index; i < m_elements.size(); ++i) {
       m_elements[i]->setName("_" + std::to_string(i));
    }
  }

  /**
   * @brief Current size of the array.
   */
  size_t size() const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");
    return m_elements.size();
  }

  void addChild(std::shared_ptr<NamedObject> child) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");
    
    NamedObject::addChild(child);
    
    auto casted = std::dynamic_pointer_cast<T>(child);
    if (casted) {
      if (std::find(m_elements.begin(), m_elements.end(), casted) == m_elements.end()) {
        m_elements.push_back(casted);
      }
    }
  }

  void removeChild(const std::string &name) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");

    std::string nameCopy = name;
    auto child = this->getChild(nameCopy);
    if (child) {
        NamedObject::removeChild(nameCopy);
        auto it = std::find(m_elements.begin(), m_elements.end(), child);
        if (it != m_elements.end()) {
            m_elements.erase(it);
        }
    }
  }

  // Iterators over elements vector
  auto begin() { return m_elements.begin(); }
  auto end() { return m_elements.end(); }
  auto begin() const { return m_elements.begin(); }
  auto end() const { return m_elements.end(); }

  std::shared_ptr<NamedObject> clone() const override {
    return NamedArray<T>::create(getName());
  }

  // Constructor
  NamedArray(const std::string &name) : NamedObject(name) {}

private:
  std::vector<std::shared_ptr<T>> m_elements;
  mutable std::recursive_timed_mutex m_arrayMutex;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDARRAY_HPP
