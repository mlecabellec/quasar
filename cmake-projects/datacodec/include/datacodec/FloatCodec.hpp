/**
 * @file FloatCodec.hpp
 * @brief Template codec implementation for IEEE floating-point NamedFloatingPoint values.
 * @details Provides endianness-aware ICodec specialization mapping between BitBufferSlice views
 * and quasar::named::NamedFloatingPoint<T> objects.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Satisfies file-level Doxygen and traceability standards (CS-0090.1, CS-0090.2).
 */
#ifndef DATACODEC_FLOATCODEC_HPP
#define DATACODEC_FLOATCODEC_HPP

// Include base codec interface and NamedFloatingPoint target wrapper.
#include "datacodec/ICodec.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
// Standard smart pointer, type trait, exception, and string headers.
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <string>

/**
 * @brief Namespace for Quasar binary and schema-driven data codecs and mappers.
 * @details Contains template codec specialization FloatCodec<T> for floating-point values.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Encloses FloatCodec within namespace datacodec (CS-0010.33).
 */
namespace datacodec {

/**
 * @class FloatCodec
 * @brief Codec for encoding/decoding floating-point values.
 * @details Validates floating-point template instantiation at compile time and manages fixed
 * sizeof(T) * 8 bit-width encoding/decoding contracts for NamedFloatingPoint<T>.
 * @tparam T Floating-point C++ type (float, double, or long double) handled by this codec.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @exposed Public template codec used in binary and JSON schema definitions.
 * @compliance Documents template parameter, contracts, invariants, and error containment (CS-0090.1..CS-0090.9).
 * @invariant m_bitSize equals sizeof(T) * 8 for the lifetime of the codec.
 */
template <typename T> class FloatCodec : public ICodec {
public:
  // Enforce at compile time that T is a standard floating-point representation.
  static_assert(std::is_floating_point<T>::value,
                "FloatCodec only supports floating-point types");

  /**
   * @brief Constructs a FloatCodec with bit size derived from sizeof(T) * 8.
   * @details Initializes fixed bit width for T and records wire byte-order preference.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public constructor for floating-point schema fields.
   * @compliance Documents constructor parameter and postconditions (CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.9).
   * @param[in] isBigEndian
   *   Justification: Selects big-endian (true) vs little-endian (false) byte order metadata.
   *   Usage: Stored in m_isBigEndian to configure floating-point serialization ordering.
   *   Consumed: Copied by value into member m_isBigEndian.
   *   Prerequisites: Boolean flag (defaults to true for big-endian).
   * @pre isBigEndian is a valid boolean byte-order selector (true for big-endian, false for little-endian).
   * @post getBitSize() == sizeof(T) * 8.
   */
  FloatCodec(bool isBigEndian = true)
      : m_bitSize(sizeof(T) * 8), m_isBigEndian(isBigEndian) {}

  /**
   * @brief Decodes a NamedFloatingPoint<T> object from the provided BitBufferSlice.
   * @details Ensures buffer holds at least sizeof(T) * 8 bits before constructing a
   * NamedFloatingPoint<T> result object.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public ICodec::decode implementation for floating-point fields.
   * @compliance Documents bounds validation, placeholder value return, and error containment (CS-0090.1..CS-0090.9).
   * @param[in] buffer
   *   Justification: Source binary slice containing IEEE floating-point bits.
   *   Usage: Checked against m_bitSize prior to extraction.
   *   Consumed: Read-only reference; underlying slice is neither modified nor stored.
   *   Prerequisites: buffer.size() >= m_bitSize.
   * @return Shared pointer to a newly created quasar::named::NamedFloatingPoint<T>.
   *   Justification: Wraps floating-point payload inside Quasar's NamedObject hierarchy.
   *   Usage: Attached to containers or cast via dynamic_pointer_cast<NamedFloatingPoint<T>>.
   *   Consumed: Ownership transferred to caller via std::shared_ptr.
   *   Prerequisites: buffer.size() >= m_bitSize.
   *   Limitations: Placeholder extraction initializes value to 0.0 pending raw bit decoding.
   *   Lifecycle: Heap-allocated via NamedFloatingPoint<T>::create; destroyed when caller shared_ptrs release.
   *   Modification / Deletion: Caller owns and may update, rename, or discard the instance.
   *   Specific Values: Non-null NamedObject pointer wrapping NamedFloatingPoint<T> on success.
   *   Error Behavior: Throws std::out_of_range if buffer.size() < m_bitSize.
   * @pre buffer.size() >= m_bitSize.
   * @post Returns a non-null std::shared_ptr dynamically castable to NamedFloatingPoint<T>.
   * @throws std::out_of_range when buffer contains fewer than m_bitSize bits.
   * @containment
   *   Approach: Validates slice width before allocating NamedFloatingPoint<T>.
   *   Limitations: Placeholder returns 0.0 without unpacking raw IEEE-754 bits yet.
   */
  std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const override {
    // Reject slices shorter than sizeof(T) * 8 bits.
    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for FloatCodec");
    }
    T value = 0.0;
    return quasar::named::NamedFloatingPoint<T>::create("decoded", value);
  }

  /**
   * @brief Returns the canonical codec category string "FloatingPoint".
   * @details Allows schema mappers such as JsonMapper to map JSON numeric tokens into floating-point nodes.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public codec category accessor.
   * @compliance Documents return contract and exception containment (CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9).
   * @return Constant string "FloatingPoint".
   *   Justification: Identifies floating-point fields for dynamic mapper type dispatch.
   *   Usage: Matched against "FloatingPoint" in JsonMapper::mapContainerStreaming.
   *   Consumed: Returned by value as an independent std::string.
   *   Prerequisites: None; stateless read.
   *   Limitations: Does not distinguish float vs double precision in category label.
   *   Lifecycle: Value-returned std::string owned by caller.
   *   Modification / Deletion: Caller may read or modify its copy freely.
   *   Specific Values: Always returns "FloatingPoint".
   *   Error Behavior: Throws std::bad_alloc only if string heap allocation fails.
   * @post Returned string equals "FloatingPoint".
   * @throws std::bad_alloc under memory exhaustion during std::string allocation.
   * @containment
   *   Approach: Returns literal string directly without mutating codec state.
   *   Limitations: Relies on std::string memory allocation for the returned string value.
   */
  std::string getCodecType() const override {
      return "FloatingPoint";
  }

  /**
   * @brief Encodes a NamedFloatingPoint<T> instance into the target BitBufferSlice.
   * @details Verifies runtime type compatibility via dynamic_pointer_cast and checks buffer bit capacity.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public ICodec::encode implementation for floating-point fields.
   * @compliance Documents type/bounds checking, branches, and containment (CS-0090.1..CS-0090.9).
   * @param[in] value
   *   Justification: NamedObject carrying the NamedFloatingPoint<T> value to encode.
   *   Usage: Dynamically cast to NamedFloatingPoint<T> to validate compatibility.
   *   Consumed: Read-only shared pointer reference; object is not modified or retained.
   *   Prerequisites: Non-null std::shared_ptr to NamedFloatingPoint<T>.
   * @param[in,out] buffer
   *   Justification: Target BitBufferSlice receiving serialized floating-point bits.
   *   Usage: Checked against m_bitSize capacity prior to serialization.
   *   Consumed: Target buffer reference used for bit output.
   *   Prerequisites: buffer.size() >= m_bitSize.
   * @pre value dynamically casts to NamedFloatingPoint<T> and buffer.size() >= m_bitSize.
   * @post Validation succeeds without partial writes on invalid type or insufficient buffer length.
   * @throws std::invalid_argument if value is null or not a NamedFloatingPoint<T>.
   * @throws std::out_of_range if buffer.size() < m_bitSize.
   * @containment
   *   Approach: Performs RTTI cast and slice-size validation before writing to buffer.
   *   Limitations: Serialization body currently acts as a validated placeholder without mutating buffer bits.
   */
  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {
    std::shared_ptr<quasar::named::NamedFloatingPoint<T>> namedFloat =
        std::dynamic_pointer_cast<quasar::named::NamedFloatingPoint<T>>(value);
    // Reject null pointers or objects of non-matching NamedObject subtype.
    if (!namedFloat) {
      throw std::invalid_argument("Invalid type for FloatCodec encoding");
    }
    // Reject slices that cannot accommodate sizeof(T) * 8 bits.
    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for FloatCodec encoding");
    }
  }

  /**
   * @brief Returns the fixed encoded bit width sizeof(T) * 8.
   * @details Reports deterministic bit footprint for floating-point type T.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public fixed-size query implementing ICodec::getBitSize().
   * @compliance Documents full return contract and containment (CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9).
   * @return Fixed bit count sizeof(T) * 8.
   *   Justification: Enables schema layout planners to compute fixed field offsets.
   *   Usage: Used to allocate slices and calculate container frame sizes.
   *   Consumed: Scalar value copy; no external ownership.
   *   Prerequisites: FloatCodec constructed.
   *   Limitations: Assumes byte width of 8 bits per byte matching IEEE storage sizes.
   *   Lifecycle: Scalar size_t scoped to caller expression.
   *   Modification / Deletion: Caller owns returned value copy.
   *   Specific Values: sizeof(T) * 8 (typically 32 for float, 64 for double).
   *   Error Behavior: Cannot fail in practice; returns cached member directly.
   * @post Return value equals sizeof(T) * 8.
   * @throws None in practice; matches non-noexcept virtual base signature.
   * @containment
   *   Approach: Read-only access to immutable member field m_bitSize.
   *   Limitations: None for valid constructed instances.
   */
  size_t getBitSize() const override { return m_bitSize; }

  /**
   * @brief Returns the fixed encoded bit size for any supplied NamedObject value.
   * @details Floating-point types always occupy sizeof(T) * 8 bits regardless of value magnitude.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public ICodec::getEncodedBitSize implementation for floating-point fields.
   * @compliance Documents unused value contract, fixed return contract, and containment (CS-0090.1..CS-0090.9).
   * @param[in] value
   *   Justification: Polymorphic value argument required by ICodec::getEncodedBitSize.
   *   Usage: Ignored via (void) cast because floating-point width is compile-time fixed.
   *   Consumed: Neither read nor stored.
   *   Prerequisites: Any shared_ptr (including nullptr, since unused).
   * @return Fixed bit width m_bitSize (sizeof(T) * 8).
   *   Justification: Conforms floating-point fields to uniform dynamic/fixed buffer-sizing API.
   *   Usage: Queried prior to slice allocation or encoding.
   *   Consumed: Plain scalar value copy.
   *   Prerequisites: FloatCodec constructed.
   *   Limitations: Does not validate whether value actually holds NamedFloatingPoint<T>.
   *   Lifecycle: Scalar size_t scoped to caller expression.
   *   Modification / Deletion: Caller owns scalar copy.
   *   Specific Values: Always returns sizeof(T) * 8.
   *   Error Behavior: Cannot fail; returns immutable scalar m_bitSize.
   * @post Return value equals m_bitSize.
   * @throws None in practice; matches non-noexcept virtual base signature.
   * @pre value may be any shared_ptr<quasar::named::NamedObject> (unused because width is compile-time fixed).
   * @containment
   *   Approach: Suppresses unused argument warning and returns immutable fixed bit width.
   *   Limitations: Type validation is deferred to encode().
   */
  size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const override {
    // Cast unused parameter to void because floating-point width is fixed.
    (void)value;
    return m_bitSize;
  }

private:
  /**
   * @brief Fixed bit width sizeof(T) * 8 of the floating-point wire format.
   * @details Initialized in constructor and used to validate buffer slice dimensions.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @compliance Private immutable configuration field documented per CS-0090.1 and CS-0090.2.
   */
  size_t m_bitSize;
  /**
   * @brief Endianness flag selecting big-endian (true) or little-endian (false) byte layout.
   * @details Controls wire byte-order interpretation relative to host architecture.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @compliance Private immutable configuration field documented per CS-0090.1 and CS-0090.2.
   */
  bool m_isBigEndian;
};

} // namespace datacodec

#endif // DATACODEC_FLOATCODEC_HPP
