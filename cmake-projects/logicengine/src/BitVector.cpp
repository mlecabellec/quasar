/**
 * @file BitVector.cpp
 * @brief Implementation of BitVector utility.
 */

#include "quasar/logic/BitVector.hpp"
#include <algorithm>
#include <stdexcept>

namespace quasar::logic {

BitVector::BitVector(std::size_t size) : m_size(size) {
    const std::size_t wordCount = (size + BITS_PER_WORD - 1) / BITS_PER_WORD;
    m_words.resize(wordCount, 0);
}

void BitVector::resize(std::size_t size) {
    const std::size_t wordCount = (size + BITS_PER_WORD - 1) / BITS_PER_WORD;
    m_words.resize(wordCount, 0);
    m_size = size;
    
    // Mask out bits beyond the new size in the last word
    if (size > 0 && (size % BITS_PER_WORD) != 0) {
        const std::uint64_t mask = (1ULL << (size % BITS_PER_WORD)) - 1;
        m_words.back() &= mask;
    }
}

void BitVector::set(std::size_t index, bool value) {
    if (index >= m_size) {
        throw std::out_of_range("BitVector index out of range");
    }
    const std::size_t wordIdx = index / BITS_PER_WORD;
    const std::size_t bitIdx = index % BITS_PER_WORD;
    if (value) {
        m_words[wordIdx] |= (1ULL << bitIdx);
    } else {
        m_words[wordIdx] &= ~(1ULL << bitIdx);
    }
}

bool BitVector::get(std::size_t index) const {
    if (index >= m_size) {
        throw std::out_of_range("BitVector index out of range");
    }
    const std::size_t wordIdx = index / BITS_PER_WORD;
    const std::size_t bitIdx = index % BITS_PER_WORD;
    return (m_words[wordIdx] & (1ULL << bitIdx)) != 0;
}

void BitVector::clear() {
    std::fill(m_words.begin(), m_words.end(), 0ULL);
}

BitVector BitVector::operator&(const BitVector& other) const {
    const std::size_t minSize = std::min(m_size, other.m_size);
    BitVector result(minSize);
    const std::size_t wordCount = result.m_words.size();
    for (std::size_t i = 0; i < wordCount; ++i) {
        result.m_words[i] = m_words[i] & other.m_words[i];
    }
    return result;
}

BitVector BitVector::operator|(const BitVector& other) const {
    const std::size_t maxSize = std::max(m_size, other.m_size);
    BitVector result(maxSize);
    const std::size_t wordCount = std::min(m_words.size(), other.m_words.size());
    for (std::size_t i = 0; i < wordCount; ++i) {
        result.m_words[i] = m_words[i] | other.m_words[i];
    }
    // Copy remaining words from the larger vector if any
    if (m_words.size() > other.m_words.size()) {
        for (std::size_t i = wordCount; i < m_words.size(); ++i) {
            result.m_words[i] = m_words[i];
        }
    } else if (other.m_words.size() > m_words.size()) {
        for (std::size_t i = wordCount; i < other.m_words.size(); ++i) {
            result.m_words[i] = other.m_words[i];
        }
    }
    return result;
}

bool BitVector::operator==(const BitVector& other) const {
    if (m_size != other.m_size) {
        return false;
    }
    return m_words == other.m_words;
}

bool BitVector::operator!=(const BitVector& other) const {
    return !(*this == other);
}

bool BitVector::any() const {
    // [CS-0010.34] auto forbidden.
    return std::any_of(m_words.begin(), m_words.end(), [](std::uint64_t word) {
        return word != 0;
    });
}

std::size_t BitVector::size() const {
    return m_size;
}

} // namespace quasar::logic
