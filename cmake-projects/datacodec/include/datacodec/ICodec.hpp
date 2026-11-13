/**
 * @file ICodec.hpp
 * @brief Interface for binary data codecs.
 * @details Defines the abstract polymorphic ICodec contract used to serialize NamedObject values
 * into BitBufferSlice views and deserialize binary payloads back into NamedObject instances.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Establishes file-level traceability and interface contract documentation under CS-0090.1 and CS-0090.2.
 */

#ifndef DATACODEC_ICODEC_HPP
#define DATACODEC_ICODEC_HPP

// Coretypes bit-buffer slice, named object hierarchy, and standard smart pointer headers.
#include "quasar/coretypes/BitBufferSlice.hpp"
#include "quasar/named/NamedObject.hpp"
#include <memory>

/**
 * @brief Namespace for Quasar binary and schema-driven data codecs and mappers.
 * @details Houses the ICodec abstraction, primitive codec templates, hierarchical schema definitions,
 * and binary/JSON mappers linking transport buffers to NamedObject trees.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Encloses all datacodec declarations inside a dedicated namespace (CS-0010.33).
 */
namespace datacodec {

/**
 * @class ICodec
 * @brief Abstract interface for encoding and decoding values to/from binary buffers.
 * @details This interface defines the contract for transforming between raw binary
 * representations (accessed via BitBufferSlice) and high-level object
 * representations (NamedObject instances). This is fundamental for defining how
 * structured data, used in EtherCAT mailboxes (e.g., CoE, FoE, EoE), Process Data
 * Objects (PDOs), and configuration parameters, is converted to and from its
 * binary representation. This directly supports features like FE-0040.4 and
 * FE-0040.5 by providing the mechanism for data serialization and deserialization.
 *
 * @par Contribution to FE-0020
 * This interface's `decode` and `encode` methods operate directly on
 * `quasar::named::NamedObject` instances or derivatives. This establishes the core
 * mechanism for interacting with the named object hierarchy, aligning with the
 * foundational requirement of FE-0020 to provide a robust framework for named objects.
 *
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020, FE-0040
 * @exposed Public codec abstraction implemented by primitive and middleware codecs across Quasar.
 * @compliance Enforces complete Doxygen coverage, parameter/return contracts, error containment,
 * and behavioral predicates on all virtual codec entry points (CS-0090.1..CS-0090.9).
 * @invariant Concrete codecs remain stateless with respect to encoded payloads across concurrent calls.
 */
class ICodec {
public:
  /**
   * @brief Virtual destructor ensuring safe polymorphic deletion through base pointers.
   * @details Defaulted virtual destructor allowing derived codec objects managed via std::shared_ptr
   * or std::unique_ptr to clean up derived members cleanly.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public polymorphic destructor for ICodec hierarchy.
   * @compliance Provides documented virtual destruction preserving base-pointer lifecycle safety (CS-0090.1, CS-0090.2, CS-0060.5).
   */
  virtual ~ICodec() = default;

  /**
   * @brief Decodes a value from the buffer.
   * @details Reads bits from the provided buffer slice and constructs a corresponding
   * NamedObject. The type of the returned object depends on the specific codec
   * implementation (e.g., NamedInteger, NamedFloatingPoint, NamedString).
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020, FE-0040
   * @exposed Public decoding entry point invoked by schema mappers and transport parsers.
   * @compliance Documents slice precondition, ownership transfer, and error containment (CS-0090.1..CS-0090.9).
   * @param[in] buffer
   *   Justification: Source binary slice containing the encoded representation to parse.
   *   Usage: Inspected for bit size and bit contents to construct the decoded NamedObject.
   *   Consumed: Read-only access; underlying buffer storage is neither modified nor retained.
   *   Prerequisites: Buffer slice must contain at least getBitSize() bits for fixed-width codecs.
   * @return A shared pointer to the newly constructed NamedObject representing the decoded value.
   *   Justification: Polymorphic ownership handle decoupled from concrete codec subtype.
   *   Usage: Attached to a parent NamedObject tree or inspected via dynamic_pointer_cast.
   *   Consumed: Ownership shared with caller via std::shared_ptr; no internal reference retained by codec.
   *   Prerequisites: Input buffer slice satisfies codec size and format requirements.
   *   Limitations: Fixed-size codecs fail when buffer slice is shorter than getBitSize().
   *   Lifecycle: Heap-allocated via NamedObject factory; freed when caller shared_ptr count drops to zero.
   *   Modification / Deletion: Caller owns the returned instance and may mutate, reparent, or release it.
   *   Specific Values: Non-null pointer to a concrete NamedObject subclass on success.
   *   Error Behavior: Throws std::out_of_range or std::runtime_error on truncated or malformed buffers.
   * @pre buffer refers to a valid BitBufferSlice sufficient for this codec.
   * @post Returns a non-null std::shared_ptr<quasar::named::NamedObject> on success.
   * @throws std::out_of_range if buffer lacks required bits; std::runtime_error if underlying data is invalid.
   * @containment
   *   Approach: Concrete codecs validate buffer bounds before extraction and raise typed standard exceptions.
   *   Limitations: Pure virtual contract delegates exact bounds checks to each derived codec implementation.
   */
  virtual std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const = 0;

  /**
   * @brief Returns the canonical type identifier of values this codec handles.
   * @details Supplies a human-readable category string (e.g., "Integer", "FloatingPoint", "String")
   * used by schema mappers such as JsonMapper to select matching runtime NamedObject types.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public codec introspection method used by JSON and binary mappers.
   * @compliance Documents full return contract and error behavior for type-tag query (CS-0090.1..CS-0090.9).
   * @return Canonical codec category string identifying handled NamedObject value kinds.
   *   Justification: Enables runtime schema dispatch without RTTI coupling in callers.
   *   Usage: Compared against expected mapper categories ("Integer", "FloatingPoint", "String", etc.).
   *   Consumed: Returned by value; caller owns the string copy independently.
   *   Prerequisites: Codec object is validly constructed.
   *   Limitations: String taxonomy is convention-based across built-in codecs.
   *   Lifecycle: Stack/value-returned std::string freed when caller variable leaves scope.
   *   Modification / Deletion: Caller may inspect or modify its returned string copy freely.
   *   Specific Values: Standard type labels such as "Integer", "FloatingPoint", or "String".
   *   Error Behavior: May throw std::bad_alloc if string construction requires heap allocation and memory is exhausted.
   * @post Returned string is non-empty for standard value codecs.
   * @throws std::bad_alloc if string allocation fails; delegates in wrapper codecs may propagate inner errors.
   * @containment
   *   Approach: Returns immutable literal category names by value with no internal state mutation.
   *   Limitations: Wrapper codecs (such as TransformCodec) must faithfully reflect or override inner codec type.
   */
  virtual std::string getCodecType() const = 0;

  /**
   * @brief Encodes a value into the target binary buffer slice.
   * @details Validates the runtime NamedObject subtype and destination slice capacity, then serializes
   * the value from the provided NamedObject into the buffer slice.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020, FE-0040
   * @exposed Public encoding entry point invoked by schema mappers and transport writers.
   * @compliance Documents runtime type check, bounds enforcement, and error containment (CS-0090.1..CS-0090.9).
   * @param[in] value
   *   Justification: NamedObject carrying the value to serialize into binary form.
   *   Usage: Dynamically cast to the expected NamedObject derivative and read for serialization.
   *   Consumed: Read-only shared pointer reference; object value is not modified or retained.
   *   Prerequisites: Must be non-null and hold the concrete NamedObject subtype expected by this codec.
   * @param[in,out] buffer
   *   Justification: Destination BitBufferSlice receiving serialized bit data.
   *   Usage: Mutated in place across the bit span assigned to this encoded value.
   *   Consumed: Target bit range is overwritten; caller retains slice and backing buffer ownership.
   *   Prerequisites: Must have size() >= getEncodedBitSize(value) bits.
   * @pre value is compatible with this codec and buffer has sufficient capacity.
   * @post Serialized representation of value is written into buffer when no exception is thrown.
   * @throws std::invalid_argument if value is null or incompatible with this codec.
   * @throws std::out_of_range if buffer is smaller than the required bit length.
   * @containment
   *   Approach: Implementations validate NamedObject type via dynamic_pointer_cast and buffer length
   *   before mutating any bits in buffer.
   *   Limitations: Partial writes may occur only if a custom derived codec writes incrementally before failing.
   */
  virtual void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
                      quasar::coretypes::BitBufferSlice &buffer) const = 0;

  /**
   * @brief Gets the fixed size of the encoded data in bits.
   * @details Reports the deterministic bit width of fixed-size codecs or zero when encoded length
   * varies per payload (such as dynamic-length strings).
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public layout query used by schema offset computation and mappers.
   * @compliance Documents fixed-vs-dynamic return contract (CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9).
   * @return Number of bits occupied by the encoded format, or 0 if the codec size is dynamic.
   *   Justification: Allows frame builders and slice calculators to reserve fixed bit widths statically.
   *   Usage: Passed to BitBufferSlice slicing or checked against 0 to detect variable-length fields.
   *   Consumed: Plain scalar value copy; caller holds no external resource.
   *   Prerequisites: Codec instance initialized with valid configuration parameters.
   *   Limitations: Returns 0 for dynamic codecs, requiring callers to invoke getEncodedBitSize(value).
   *   Lifecycle: Value-type size_t destroyed at end of caller expression.
   *   Modification / Deletion: Caller may copy or accumulate the returned scalar freely.
   *   Specific Values: Positive bit count (> 0) for fixed codecs; 0 for dynamic-length codecs.
   *   Error Behavior: Fixed implementations do not fail; wrapper codecs delegate to wrapped codec.
   * @post Returns a non-negative fixed bit count or 0 for dynamic sizing.
   * @throws std::runtime_error if a delegating codec wrapper holds an invalid inner codec state.
   * @containment
   *   Approach: Pure const inspection query without side effects or shared-state mutation.
   *   Limitations: Does not reflect runtime payload size when returning 0 for variable-length types.
   */
  virtual size_t getBitSize() const = 0;

  /**
   * @brief Gets the exact encoded size in bits required for a specific value instance.
   * @details Computes the actual serialized bit footprint of value, supporting dynamic-length types
   * such as null-terminated or variable-length strings.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public dynamic-sizing query used prior to buffer allocation and encoding.
   * @compliance Documents parameter/return contracts, edge cases, and containment (CS-0090.1..CS-0090.9).
   * @param[in] value
   *   Justification: Candidate NamedObject whose encoded payload bit count must be measured.
   *   Usage: Inspected when the codec is variable-width; may be ignored by fixed-width codecs.
   *   Consumed: Read-only shared pointer reference; object is neither mutated nor stored.
   *   Prerequisites: For variable-width codecs, should point to a compatible non-null NamedObject.
   * @return Number of bits required to encode value into a BitBufferSlice.
   *   Justification: Enables exact buffer sizing before calling encode().
   *   Usage: Compared against destination slice size() or summed for composite container frames.
   *   Consumed: Scalar value returned by copy; caller owns no heap state.
   *   Prerequisites: Codec is constructed and value is compatible when dynamic measurement is required.
   *   Limitations: Incompatible or null values may yield 0 on some dynamic codecs or throw in wrappers.
   *   Lifecycle: Scalar size_t scoped to caller expression.
   *   Modification / Deletion: Caller may use or discard the returned bit count freely.
   *   Specific Values: Fixed bit width for fixed codecs; payload-dependent bit count (>= 0) for dynamic codecs.
   *   Error Behavior: Wrapper codecs may propagate transformation exceptions; null checks yield 0 where specified.
   * @pre value is non-null when measuring variable-length encodings.
   * @post Returned bit length fits within platform size_t limits.
   * @throws std::exception if a wrapped transformer or concrete codec validates and rejects value.
   * @containment
   *   Approach: Read-only calculation that neither allocates persistent storage nor modifies value.
   *   Limitations: Returning 0 on dynamic-type mismatch leaves caller responsible for subsequent encode validation.
   */
  virtual size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const = 0;
};

} // namespace datacodec

#endif // DATACODEC_ICODEC_HPP
