/**
 * @file BitVector.hpp
 * @brief High-performance bit vector utility for logic operations.
 */

#ifndef QUASAR_LOGIC_BITVECTOR_HPP
#define QUASAR_LOGIC_BITVECTOR_HPP

#include <vector>
#include <cstdint>
#include <cstddef>

namespace quasar::logic {

/**
 * @class BitVector
 * @brief Represents a sequence of bits for logic operations.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260311-009.1.3] Implement BitVector utility.
 * - Fulfills [TSK-20260311-009.4.2] Matrix evaluation support.
 */
class BitVector {
public:
    /** @brief Constructor. 
     *  @param size Number of bits. 
     */
    explicit BitVector(std::size_t size = 0);

    /** @brief Resizes the vector. 
     *  @param size New number of bits. 
     */
    void resize(std::size_t size);

    /** @brief Sets a bit value. 
     *  @param index Bit index. 
     *  @param value New value. 
     */
    void set(std::size_t index, bool value = true);

    /** @brief Gets a bit value. 
     *  @param index Bit index. 
     *  @return Bit value. 
     */
    bool get(std::size_t index) const;

    /** @brief Clears all bits. */
    void clear();

    /** @brief Bitwise AND. 
     *  @param other Other bit vector. 
     *  @return Result bit vector. 
     */
    BitVector operator&(const BitVector& other) const;

    /** @brief Bitwise OR. 
     *  @param other Other bit vector. 
     *  @return Result bit vector. 
     */
    BitVector operator|(const BitVector& other) const;

    /** @brief Equality comparison. 
     *  @param other Other bit vector. 
     *  @return true if both vectors have same bits. 
     */
    bool operator==(const BitVector& other) const;

    /** @brief Inequality comparison. 
     *  @param other Other bit vector. 
     *  @return true if vectors are different. 
     */
    bool operator!=(const BitVector& other) const;

    /** @brief Returns true if any bit is set. 
     *  @return true if not all bits are zero. 
     */
    bool any() const;

    /** @brief Gets the number of bits. 
     *  @return bit count. 
     */
    std::size_t size() const;

private:
    /** @brief Number of bits. */
    std::size_t m_size;
    /** @brief Internal storage. */
    std::vector<std::uint64_t> m_words;

    /** @brief Static constant for word size in bits. */
    static constexpr std::size_t BITS_PER_WORD = 64;
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_BITVECTOR_HPP
