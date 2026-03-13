#ifndef SIMPLE_COLLECTION_HPP
#define SIMPLE_COLLECTION_HPP

#include <Smp/ICollection.h>
#include <Smp/CollectionIterator.h>
#include <vector>
#include <string>
#include <algorithm>

namespace sample {

/**
 * @brief A simple implementation of Smp::ICollection using std::vector.
 * @tparam T The type of elements in the collection.
 */
template <typename T>
class SimpleCollection : public virtual Smp::ICollection<T> {
public:
    using const_iterator = Smp::CollectionIterator<T>;
    using iterator = Smp::CollectionIterator<T>;

    virtual ~SimpleCollection() noexcept = default;

    /**
     * @brief Retrieve element by name.
     * @param name The name of the element.
     * @return T* The element or nullptr if not found.
     */
    T* at(Smp::String8 name) const override {
        if (!name) return nullptr;
        std::string nameStr(name);
        auto it = std::find_if(_elements.begin(), _elements.end(),
            [&nameStr](T* element) {
                return element && std::string(element->GetName()) == nameStr;
            });
        return (it != _elements.end()) ? *it : nullptr;
    }

    /**
     * @brief Retrieve element by index.
     * @param index The index of the element.
     * @return T* The element or nullptr if index out of bounds.
     */
    T* at(size_t index) const override {
        if (index < _elements.size()) {
            return _elements[index];
        }
        return nullptr;
    }

    /**
     * @brief Get the number of elements in the collection.
     * @return size_t The number of elements.
     */
    size_t size() const override { return _elements.size(); }

    /**
     * @brief Check if the collection is empty.
     * @return Smp::Bool True if empty, false otherwise.
     */
    Smp::Bool empty() const override { return _elements.empty(); }

    /**
     * @brief Get the begin iterator.
     * @return const_iterator The begin iterator.
     */
    const_iterator begin() const override { return const_iterator(*this, 0); }

    /**
     * @brief Get the end iterator.
     * @return const_iterator The end iterator.
     */
    const_iterator end() const override { return const_iterator(*this, _elements.size()); }

    /**
     * @brief Add an element to the collection.
     * @param element The element to add.
     */
    void Add(T* element) {
        if (element) {
            _elements.push_back(element);
        }
    }

private:
    std::vector<T*> _elements;
};

} // namespace sample

#endif // SIMPLE_COLLECTION_HPP
