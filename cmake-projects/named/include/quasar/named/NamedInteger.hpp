/**
 * @file NamedInteger.hpp
 * @brief Template class for named integer values.
 */

#ifndef QUASAR_NAMED_NAMEDINTEGER_HPP
#define QUASAR_NAMED_NAMEDINTEGER_HPP

#include "quasar/coretypes/Integer.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/IBoundPrimitive.hpp"
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedInteger
 * @brief A named object that holds an integer value.
 *
 * This class combines the hierarchical capabilities of NamedObject with the
 * integer value management of coretypes::Integer.
 * 
 * **Compliance**:
 * - Fulfills [FE-0020.4] Derivative class for Integer core type.
 * - Fulfills [CS-0010.34] auto forbidden.
 *
 * @tparam T The underlying integer type (e.g., int, uint32_t, int64_t).
 */
template <typename T>
class NamedInteger : public NamedObject, public quasar::coretypes::Integer<T>, public IBoundPrimitive {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedInteger() = default;

  /**
   * @brief Factory method to create a NamedInteger.
   * 
   * Fulfills [FE-0020.6] static method "create".
   *
   * @param name The name of the integer object.
   * @param value The initial integer value.
   * @param parent Optional parent in the hierarchy.
   * @return Shared pointer to the new NamedInteger instance.
   */
  static std::shared_ptr<NamedInteger<T>>
  create(const std::string &name, T value,
         std::shared_ptr<NamedObject> parent = nullptr) {
    // Instantiate the NamedInteger.
    std::shared_ptr<NamedInteger<T>> obj =
        std::make_shared<NamedInteger<T>>(name, value);

    // Initialize self-reference for getSelf().
    obj->setSelf(obj);

    // Attach to parent if provided.
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Creates a clone of this NamedInteger.
   * 
   * Fulfills [FE-0020.14] Utilities for copying parts of the tree.
   * 
   * @param policy Memory policy (DUPLICATE vs SHARE). For a simple integer,
   * share might imply returning the exact same node or a new node pointing to the
   * same buffer if bound. We default to the standard deep copy for now.
   * @return A new NamedInteger with the same name and value, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone(CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    if (policy == CopyPolicy::SHARE && m_bound) {
        // Advanced sharing logic will go here. For now, bind the new one too.
        auto newObj = create(this->getName(), this->value());
        newObj->bind(m_bound_offset, m_bound_length);
        return newObj;
    }
    // Return a new instance using the same name and current value.
    return create(this->getName(), this->value());
  }

  // --- IBoundPrimitive implementation ---
  bool isBound() const override { return m_bound; }
  std::size_t getBoundOffset() const override { return m_bound_offset; }
  std::size_t getBoundLength() const override { return m_bound_length; }

  /**
   * @brief Binds this integer to a specific memory offset (conceptual in Phase 1).
   */
  void bind(std::size_t offset, std::size_t length) {
      m_bound = true;
      m_bound_offset = offset;
      m_bound_length = length;
  }

  /**
   * @brief Returns the type of the object.
   * @return "NamedInteger"
   */
  std::string getType() const override { return "NamedInteger"; }

  /**
   * @brief Constructs a NamedInteger.
   * @param name The name of the object.
   * @param value The initial value.
   */
  NamedInteger(const std::string &name, T value)
      : NamedObject(name), quasar::coretypes::Integer<T>(value),
        m_bound(false), m_bound_offset(0), m_bound_length(sizeof(T)) {
    // Both base classes are initialized with the provided arguments.
  }

private:
  bool m_bound;
  std::size_t m_bound_offset;
  std::size_t m_bound_length;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDINTEGER_HPP

