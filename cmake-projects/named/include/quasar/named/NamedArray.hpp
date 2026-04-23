/**
 * @file NamedArray.hpp
 * @brief Template class for named array collections.
 */

#ifndef QUASAR_NAMED_NAMEDARRAY_HPP
#define QUASAR_NAMED_NAMEDARRAY_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <vector>
#include <stdexcept>
#include <string>
#include <mutex>
#include <algorithm>
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedArray
 * @brief A NamedObject that manages a collection of elements.
 *
 * Managing elements as NamedObject children allows iterating over them
 * generically, while elements are named according to their index ("0", "1", ...).
 * 
 * **Compliance**:
 * - Fulfills [FE-0110.2.1] Named Collections: NamedArray<T>.
 * - Fulfills [CS-0010.34] auto forbidden.
 *
 * @tparam T The type of elements, must derive from NamedObject.
 */
template <typename T>
class NamedArray : public NamedObject {
  static_assert(std::is_base_of<NamedObject, T>::value, "T must derive from NamedObject");
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~NamedArray() = default;

  /**
   * @brief Factory method.
   * @param name The name of the collection.
   * @param parent The optional parent.
   * @return A shared pointer to the new array.
   */
  static std::shared_ptr<NamedArray<T>> create(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    std::shared_ptr<NamedArray<T>> obj = std::make_shared<NamedArray<T>>(name);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Returns the type of the object.
   * @return "NamedArray"
   */
  std::string getType() const override { return "NamedArray"; }

  /**
   * @brief Access element by index.
   * @param index The index.
   * @return The element.
   */
  std::shared_ptr<T> get(size_t index) const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");
    if (index >= m_elements.size()) {
       throw std::out_of_range("Index out of bounds");
    }
    return m_elements[index];
  }

  /**
   * @brief Appends an element to the array.
   * Its name is changed to the index number.
   * @param item The item to push.
   */
  void push_back(std::shared_ptr<T> item) {
    if (!item) throw std::invalid_argument("Cannot insert null item in NamedArray");

    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");

    size_t newIndex = m_elements.size();
    std::string newName = "_" + std::to_string(newIndex);
    
    item->setName(newName);
    
    // Add to NamedObject child tree
    addChild(item);
  }

  /**
   * @brief Removes the element at the specified index and shifts subsequent elements.
   * @param index The index to remove.
   */
  void removeAt(size_t index) {
    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");

    if (index >= m_elements.size()) {
       throw std::out_of_range("Index out of bounds");
    }

    std::shared_ptr<T> item = m_elements[index];
    
    // Remove from NamedObject child tree
    removeChild(item->getName());

    // Rename subsequent elements to keep index names contiguous
    // [CS-0010.37] Loop hard limit.
    std::size_t iterations = 0;
    for (size_t i = index; i < m_elements.size(); ++i) {
       if (++iterations > config::HARD_LIMIT_ITERATIONS) {
           throw std::runtime_error("Hard limit reached in NamedArray::removeAt");
       }
       m_elements[i]->setName("_" + std::to_string(i));
    }
  }

  /**
   * @brief Current size of the array.
   * @return The size.
   */
  size_t size() const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");
    return m_elements.size();
  }

  /**
   * @brief Internal helper to add a child.
   * @param child The child to add.
   */
  void addChild(std::shared_ptr<NamedObject> child) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");
    
    NamedObject::addChild(child);
    
    std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(child);
    if (casted) {
      if (std::find(m_elements.begin(), m_elements.end(), casted) == m_elements.end()) {
        m_elements.push_back(casted);
      }
    }
  }

  /**
   * @brief Internal helper to remove a child by name.
   * @param name Name of the child to remove.
   */
  void removeChild(const std::string &name) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_arrayMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedArray lock");

    std::string nameCopy = name;
    std::shared_ptr<NamedObject> child = this->getChild(nameCopy);
    if (child) {
        NamedObject::removeChild(nameCopy);
        typename std::vector<std::shared_ptr<T>>::iterator it = std::find(m_elements.begin(), m_elements.end(), child);
        if (it != m_elements.end()) {
            m_elements.erase(it);
        }
    }
  }

  /**
   * @brief Iterator to the beginning.
   * @return Iterator.
   */
  typename std::vector<std::shared_ptr<T>>::iterator begin() { return m_elements.begin(); }
  /**
   * @brief Iterator to the end.
   * @return Iterator.
   */
  typename std::vector<std::shared_ptr<T>>::iterator end() { return m_elements.end(); }
  /**
   * @brief Const iterator to the beginning.
   * @return Iterator.
   */
  typename std::vector<std::shared_ptr<T>>::const_iterator begin() const { return m_elements.begin(); }
  /**
   * @brief Const iterator to the end.
   * @return Iterator.
   */
  typename std::vector<std::shared_ptr<T>>::const_iterator end() const { return m_elements.end(); }

  /**
   * @brief Clones the array.
   * @return Cloned object.
   */
  std::shared_ptr<NamedObject> clone([[maybe_unused]] CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    return NamedArray<T>::create(getName());
  }

  /**
   * @brief Constructor.
   * @param name The name.
   */
  explicit NamedArray(const std::string &name) : NamedObject(name) {}

protected:
  /** @brief Internal elements storage. */
  std::vector<std::shared_ptr<T>> m_elements;
  /** @brief Mutex for protection. */
  mutable std::recursive_timed_mutex m_arrayMutex;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDARRAY_HPP

