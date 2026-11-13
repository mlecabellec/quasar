/**
 * @file AdvancedSchema.hpp
 * @brief Advanced schema definitions including ConditionalFieldDef and TransformCodec.
 * @details Provides runtime-conditional field definitions and composable middleware codec wrappers
 * for context-sensitive parsing and value transformation atop Schema.hpp.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020, FE-0040
 * @compliance Enforces file-level Doxygen and traceability standards (CS-0090.1, CS-0090.2).
 */

#ifndef DATACODEC_ADVANCED_SCHEMA_HPP
#define DATACODEC_ADVANCED_SCHEMA_HPP

// Include core schema FieldDef/ContainerDef definitions and standard std::function wrapper.
#include "datacodec/Schema.hpp"
#include <functional>

/**
 * @brief Namespace for Quasar binary and schema-driven data codecs and mappers.
 * @details Houses advanced schema decorators ConditionalFieldDef and TransformCodec.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Encloses declarations inside namespace datacodec (CS-0010.33).
 */
namespace datacodec {

/**
 * @class ConditionalFieldDef
 * @brief A field that is only present if a condition is met.
 * @details Models EtherCAT data structures with optional fields or fields whose presence
 * depends on configuration or previously decoded values, contributing to flexible SDO/PDO
 * representations for FE-0040.
 *
 * @par Contribution to FE-0020
 * Inherits from FieldDef, which inherits from quasar::named::NamedObject (FE-0020.4). Its
 * `isPresent` method takes a `quasar::named::NamedObject*` as context, demonstrating interaction
 * with the named object hierarchy for conditional logic.
 *
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020, FE-0040
 * @exposed Public optional-field schema descriptor consumed by BinaryMapper.
 * @compliance Documents alias, factory, constructor, predicate method, and containment (CS-0090.1..CS-0090.9).
 * @invariant Empty predicate m_condition unconditionally treats the field as present.
 */
class ConditionalFieldDef : public FieldDef {
public:
  /**
   * @brief Callable predicate signature evaluating conditional field presence from decoded context.
   * @details Accepts a read-only NamedObject context pointer and returns true if the field exists.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public predicate alias for conditional schema configuration.
   * @compliance Public type alias documented per CS-0090.1 and CS-0090.2.
   */
  using Predicate = std::function<bool(const quasar::named::NamedObject *)>;

  /**
   * @brief Factory creating a shared ConditionalFieldDef instance.
   * @details Allocates a ConditionalFieldDef via std::make_shared forwarding name, codec, bitOffset,
   * and presence predicate.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public factory helper for constructing conditional schema fields.
   * @compliance Documents parameter/return contracts and allocation containment (CS-0090.1..CS-0090.9).
   * @param[in] name
   *   Justification: Field name registered on the base NamedObject/FieldDef.
   *   Usage: Forwarded to ConditionalFieldDef constructor.
   *   Consumed: Copied into base NamedObject storage.
   *   Prerequisites: Valid non-empty NamedObject identifier string.
   * @param[in] codec
   *   Justification: Codec parsing or formatting this field when present.
   *   Usage: Forwarded to FieldDef base constructor.
   *   Consumed: Shared reference count incremented inside FieldDef.
   *   Prerequisites: Non-null std::shared_ptr<ICodec> expected when field can become present.
   * @param[in] bitOffset
   *   Justification: Starting bit offset of the field inside the enclosing container frame.
   *   Usage: Stored in FieldDef base m_bitOffset.
   *   Consumed: Scalar copied by value.
   *   Prerequisites: Non-negative bit index within container bounds.
   * @param[in] condition
   *   Justification: Runtime predicate deciding whether this field exists in a given context.
   *   Usage: Copied into m_condition and evaluated by isPresent().
   *   Consumed: Copied into std::function m_condition.
   *   Prerequisites: Either an empty std::function (always present) or a valid callable accepting const NamedObject*.
   * @return Shared pointer to the newly constructed ConditionalFieldDef.
   *   Justification: Allows conditional fields to be stored polymorphically inside ContainerDef::m_fields.
   *   Usage: Added to ContainerDef via addField().
   *   Consumed: Shared ownership transferred to caller via std::shared_ptr.
   *   Prerequisites: Valid name and sufficient heap memory.
   *   Limitations: Predicate captures remain caller's responsibility for thread safety and lifetime.
   *   Lifecycle: Heap-allocated via std::make_shared; freed when all shared_ptrs release.
   *   Modification / Deletion: Caller may register in ContainerDef and query isPresent().
   *   Specific Values: Non-null std::shared_ptr<ConditionalFieldDef> on success.
   *   Error Behavior: Propagates allocation or NamedObject construction exceptions.
   * @pre name obeys NamedObject rules.
   * @post Returned object stores name, codec, bitOffset, and condition.
   * @throws std::exception on memory allocation failure or invalid NamedObject name.
   * @containment
   *   Approach: Uses std::make_shared for exception-safe resource construction.
   *   Limitations: Does not invoke condition during factory creation; invalid captures fail at isPresent().
   */
  static std::shared_ptr<ConditionalFieldDef>
  create(const std::string &name, std::shared_ptr<ICodec> codec,
         size_t bitOffset, Predicate condition) {
    return std::make_shared<ConditionalFieldDef>(name, codec, bitOffset,
                                                 condition);
  }

  /**
   * @brief Constructs a ConditionalFieldDef with name, codec, bit offset, and presence predicate.
   * @details Initializes FieldDef base attributes and stores condition for runtime evaluation.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public constructor used directly or through ConditionalFieldDef::create().
   * @compliance Documents constructor parameter contracts and postcondition (CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.9).
   * @param[in] name
   *   Justification: Field identifier passed to base FieldDef.
   *   Usage: Initializes base NamedObject name.
   *   Consumed: Copied into FieldDef base storage.
   *   Prerequisites: Valid NamedObject identifier.
   * @param[in] codec
   *   Justification: Codec associated with this field when active.
   *   Usage: Forwarded to FieldDef base constructor.
   *   Consumed: Shared ownership reference stored in FieldDef.
   *   Prerequisites: Valid ICodec shared_ptr.
   * @param[in] bitOffset
   *   Justification: Bit offset of the field inside container slice.
   *   Usage: Stored in FieldDef base m_bitOffset.
   *   Consumed: Scalar copied by value.
   *   Prerequisites: Non-negative bit index.
   * @param[in] condition
   *   Justification: Optional predicate determining dynamic presence.
   *   Usage: Copied into m_condition.
   *   Consumed: Copied by value into std::function m_condition.
   *   Prerequisites: Valid callable target or empty std::function.
   * @pre name satisfies NamedObject constraints.
   * @post FieldDef metadata and m_condition are initialized.
   */
  ConditionalFieldDef(const std::string &name, std::shared_ptr<ICodec> codec,
                      size_t bitOffset, Predicate condition)
      : FieldDef(name, codec, bitOffset), m_condition(condition) {}

  /**
   * @brief Evaluates whether this field is present for the supplied container context.
   * @details Invokes m_condition(context) when a non-empty predicate is configured; otherwise returns
   * true unconditionally.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public condition-evaluation hook invoked by BinaryMapper::decode().
   * @compliance Documents context parameter, fallback behavior, branch comment, and containment (CS-0090.1..CS-0090.9).
   * @param[in] context
   *   Justification: Non-owning pointer to the partially decoded parent NamedObject context.
   *   Usage: Passed directly to m_condition when m_condition != nullptr.
   *   Consumed: Read-only raw pointer forwarded to predicate; not retained by ConditionalFieldDef.
   *   Prerequisites: Must remain valid for the duration of m_condition(context) if dereferenced by predicate.
   * @return Boolean indicating whether this field should be decoded/encoded in the given context.
   *   Justification: Enables mappers to skip absent optional fields deterministically.
   *   Usage: Evaluated in mapper loop guards prior to slicing and decoding.
   *   Consumed: Plain bool returned by value.
   *   Prerequisites: Valid context pointer matching predicate expectations when m_condition is set.
   *   Limitations: Any exception thrown by user-supplied m_condition propagates directly to caller.
   *   Lifecycle: Scalar bool destroyed at caller expression end.
   *   Modification / Deletion: Caller reads scalar result without ownership concerns.
   *   Specific Values: m_condition(context) if m_condition != nullptr; true otherwise.
   *   Error Behavior: Propagates exceptions thrown by the bound predicate callable.
   * @pre If m_condition is callable and dereferences context, context must point to a valid NamedObject.
   * @post ConditionalFieldDef remains unmodified; returns predicate truth value or true default.
   * @throws std::exception if user-supplied m_condition throws during evaluation.
   * @containment
   *   Approach: Explicitly checks m_condition != nullptr before invoking std::function, preventing std::bad_function_call.
   *   Limitations: Cannot guard against dangling context pointers or exceptions raised inside user predicate code.
   */
  bool isPresent(const quasar::named::NamedObject *context) const {
    // Evaluate configured predicate when bound; treat field as unconditionally present otherwise.
    if (m_condition != nullptr) {
      return m_condition(context);
    }
    return true;
  }

private:
  /**
   * @brief Predicate callable evaluated against parent NamedObject context to decide field presence.
   * @details When empty (nullptr), isPresent() defaults to returning true.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @compliance Private predicate member documented per CS-0090.1 and CS-0090.2.
   */
  Predicate m_condition;
};

/**
 * @class TransformCodec
 * @brief Middleware codec that applies a transformation to the decoded value.
 * @details Applies pre-processing or post-processing steps during encoding or decoding of
 * EtherCAT data such as validation, type coercion, or domain-specific transformations
 * aiding FE-0040.
 *
 * @par Contribution to FE-0020
 * Inherits from ICodec and explicitly uses `std::shared_ptr<quasar::named::NamedObject>` in
 * its transformer types and method signatures, supporting manipulation within the named object
 * framework as envisioned by FE-0020.
 *
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020, FE-0040
 * @exposed Public decorator codec wrapping an underlying ICodec with forward/reverse transforms.
 * @compliance Documents function aliases, delegated overrides, and error containment (CS-0090.1..CS-0090.9).
 * @invariant m_rawCodec, m_transform, and m_reverseTransform must remain callable for delegated operations.
 */
class TransformCodec : public ICodec {
public:
  /**
   * @brief Forward transformation function applied after raw decoding.
   * @details Transforms a raw decoded NamedObject into a derived or scaled NamedObject.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public transformer callback alias on TransformCodec.
   * @compliance Public alias documented per CS-0090.1 and CS-0090.2.
   */
  using Transformer = std::function<std::shared_ptr<quasar::named::NamedObject>(
      std::shared_ptr<quasar::named::NamedObject>)>;
  /**
   * @brief Inverse transformation function applied prior to raw encoding or bit-size measurement.
   * @details Converts a domain-level NamedObject back into the raw NamedObject accepted by m_rawCodec.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public reverse-transformer callback alias on TransformCodec.
   * @compliance Public alias documented per CS-0090.1 and CS-0090.2.
   */
  using ReverseTransformer =
      std::function<std::shared_ptr<quasar::named::NamedObject>(
          std::shared_ptr<quasar::named::NamedObject>)>;

  /**
   * @brief Constructs a TransformCodec wrapping an underlying codec with forward/reverse transforms.
   * @details Retains shared ownership of rawCodec and stores transform/reverseTransform callables.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public decorator constructor for composing middleware transformations.
   * @compliance Documents constructor parameter contracts and preconditions (CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.9).
   * @param[in] rawCodec
   *   Justification: Underlying concrete ICodec handling binary bit serialization/deserialization.
   *   Usage: Delegated to in decode(), encode(), getBitSize(), and getEncodedBitSize().
   *   Consumed: Copied into m_rawCodec (shared ownership retained).
   *   Prerequisites: Non-null std::shared_ptr<ICodec>.
   * @param[in] transform
   *   Justification: Post-decode transformation callable converting raw NamedObject values.
   *   Usage: Invoked in decode() on the result of m_rawCodec->decode().
   *   Consumed: Copied into m_transform.
   *   Prerequisites: Non-empty callable Transformer.
   * @param[in] reverseTransform
   *   Justification: Pre-encode transformation callable preparing values for m_rawCodec.
   *   Usage: Invoked in encode() and getEncodedBitSize() before delegating to m_rawCodec.
   *   Consumed: Copied into m_reverseTransform.
   *   Prerequisites: Non-empty callable ReverseTransformer.
   * @pre rawCodec != nullptr, transform != nullptr, and reverseTransform != nullptr.
   * @post TransformCodec delegates wire IO to rawCodec through the configured transforms.
   */
  TransformCodec(std::shared_ptr<ICodec> rawCodec, Transformer transform,
                 ReverseTransformer reverseTransform)
      : m_rawCodec(rawCodec), m_transform(transform),
        m_reverseTransform(reverseTransform) {}

  /**
   * @brief Decodes raw binary bits using m_rawCodec and applies m_transform to the result.
   * @details Sequentially executes m_rawCodec->decode(buffer) followed by m_transform(rawValue).
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public ICodec::decode middleware override.
   * @compliance Documents decoding pipeline, return contract, and error containment (CS-0090.1..CS-0090.9).
   * @param[in] buffer
   *   Justification: Input BitBufferSlice holding raw binary field bits.
   *   Usage: Passed directly to m_rawCodec->decode(buffer).
   *   Consumed: Read-only reference passed through to raw codec.
   *   Prerequisites: Must satisfy m_rawCodec->decode() size/format prerequisites.
   * @return Transformed NamedObject returned by m_transform.
   *   Justification: Exposes domain-adapted NamedObject seamlessly through the ICodec interface.
   *   Usage: Consumed identically to any other ICodec::decode() result.
   *   Consumed: Shared ownership returned by m_transform to caller.
   *   Prerequisites: m_rawCodec and m_transform are valid non-null callables.
   *   Limitations: Throws if m_rawCodec or m_transform is null/empty or raises an exception.
   *   Lifecycle: Heap-allocated object managed by std::shared_ptr.
   *   Modification / Deletion: Caller owns returned shared_ptr.
   *   Specific Values: Output of m_transform(m_rawCodec->decode(buffer)).
   *   Error Behavior: Propagates std::out_of_range, std::bad_function_call, or transformer exceptions.
   * @pre m_rawCodec != nullptr and m_transform is callable.
   * @post Returns transformed NamedObject without mutating buffer.
   * @throws std::exception from m_rawCodec->decode() or std::bad_function_call / transformer errors.
   * @containment
   *   Approach: Delegates raw bounds validation to m_rawCodec before invoking m_transform.
   *   Limitations: Does not check m_rawCodec or m_transform for null before invocation.
   */
  std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const override {
    // Decode raw representation first, then pass intermediate value through forward transformer.
    std::shared_ptr<quasar::named::NamedObject> rawValue =
        m_rawCodec->decode(buffer);
    return m_transform(rawValue);
  }

  /**
   * @brief Applies m_reverseTransform to value and encodes the result via m_rawCodec.
   * @details Converts domain value back into raw form before delegating binary serialization.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public ICodec::encode middleware override.
   * @compliance Documents pipeline, parameter contracts, and error propagation (CS-0090.1..CS-0090.9).
   * @param[in] value
   *   Justification: Domain-level NamedObject to convert and serialize.
   *   Usage: Passed to m_reverseTransform(value) before raw encoding.
   *   Consumed: Read-only reference passed to reverse transformer.
   *   Prerequisites: Compatible with m_reverseTransform expectations.
   * @param[in,out] buffer
   *   Justification: Destination BitBufferSlice receiving serialized raw bits.
   *   Usage: Passed to m_rawCodec->encode(rawValue, buffer).
   *   Consumed: Mutated by m_rawCodec across target field bit span.
   *   Prerequisites: Capacity sufficient for rawValue encoded bit size.
   * @pre m_rawCodec != nullptr, m_reverseTransform is callable, and buffer meets raw bit size.
   * @post Transformed raw representation is written into buffer via m_rawCodec.
   * @throws std::exception from m_reverseTransform or m_rawCodec->encode() (invalid_argument, out_of_range, etc.).
   * @containment
   *   Approach: Runs reverse transformation completely prior to delegating to m_rawCodec->encode(), avoiding buffer writes on transform failure.
   *   Limitations: Assumes m_rawCodec and m_reverseTransform are validly initialized at construction.
   */
  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {
    // Reverse-transform domain object into raw NamedObject prior to delegating wire encoding.
    std::shared_ptr<quasar::named::NamedObject> rawValue =
        m_reverseTransform(value);
    m_rawCodec->encode(rawValue, buffer);
  }

  /**
   * @brief Returns fixed bit width reported by the underlying m_rawCodec.
   * @details Delegates directly to m_rawCodec->getBitSize().
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public ICodec::getBitSize middleware override.
   * @compliance Documents delegation contract and return value semantics (CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9).
   * @return Fixed bit size of m_rawCodec, or 0 if dynamic.
   *   Justification: Exposes underlying wire footprint transparently to schema mappers.
   *   Usage: Used identically to ICodec::getBitSize().
   *   Consumed: Plain scalar value copy.
   *   Prerequisites: m_rawCodec != nullptr.
   *   Limitations: Reflects wire width of raw encoded value rather than in-memory transformed object.
   *   Lifecycle: Scalar size_t scoped to caller expression.
   *   Modification / Deletion: Caller owns scalar copy.
   *   Specific Values: Return value of m_rawCodec->getBitSize().
   *   Error Behavior: Propagates any exception thrown by m_rawCodec->getBitSize(); undefined if m_rawCodec is null.
   * @pre m_rawCodec != nullptr.
   * @post Returns m_rawCodec->getBitSize().
   * @throws std::exception if delegated m_rawCodec->getBitSize() throws.
   * @containment
   *   Approach: Read-only delegation without transforming state.
   *   Limitations: Does not perform explicit null check on m_rawCodec.
   */
  size_t getBitSize() const override { return m_rawCodec->getBitSize(); }

  /**
   * @brief Computes dynamic encoded bit size by reverse-transforming value and querying m_rawCodec.
   * @details Executes m_reverseTransform(value) and returns m_rawCodec->getEncodedBitSize(rawValue).
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public ICodec::getEncodedBitSize middleware override.
   * @compliance Documents reverse-transform sizing pipeline and error containment (CS-0090.1..CS-0090.9).
   * @param[in] value
   *   Justification: Domain-level NamedObject whose wire bit footprint must be measured.
   *   Usage: Passed to m_reverseTransform before measuring with m_rawCodec->getEncodedBitSize().
   *   Consumed: Read-only shared_ptr forwarded to m_reverseTransform.
   *   Prerequisites: Value compatible with m_reverseTransform.
   * @return Wire bit count required to encode reverse-transformed value.
   *   Justification: Ensures dynamic transforms altering payload length report accurate wire sizes.
   *   Usage: Used prior to buffer allocation or encode().
   *   Consumed: Plain scalar size_t copy.
   *   Prerequisites: m_rawCodec != nullptr and m_reverseTransform is callable.
   *   Limitations: Invokes m_reverseTransform during sizing, so stateful or expensive transforms incur extra calls.
   *   Lifecycle: Scalar size_t scoped to caller expression.
   *   Modification / Deletion: Caller owns scalar copy.
   *   Specific Values: Exact value returned by m_rawCodec->getEncodedBitSize(m_reverseTransform(value)).
   *   Error Behavior: Propagates std::bad_function_call or transformer/codec exceptions.
   * @pre m_rawCodec != nullptr and m_reverseTransform is callable.
   * @post Returns exact bit count required by encode(value, buffer).
   * @throws std::exception if m_reverseTransform or m_rawCodec->getEncodedBitSize() throws.
   * @containment
   *   Approach: Executes side-effect-free measurement delegation without modifying buffer state.
   *   Limitations: Assumes m_reverseTransform is pure/idempotent across sizing and encoding calls.
   */
  size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const override {
    // Convert to raw representation first so wire length reflects post-reverseTransform payload.
    std::shared_ptr<quasar::named::NamedObject> rawValue =
        m_reverseTransform(value);
    return m_rawCodec->getEncodedBitSize(rawValue);
  }

private:
  /**
   * @brief Underlying raw binary codec performing bit-level decode/encode operations.
   * @details Shared pointer retained for the lifetime of the TransformCodec decorator.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @compliance Private delegated codec member documented per CS-0090.1 and CS-0090.2.
   */
  std::shared_ptr<ICodec> m_rawCodec;
  /**
   * @brief Post-decode transformation applied to raw decoded NamedObject results.
   * @details Invoked by decode() after m_rawCodec returns a raw NamedObject.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @compliance Private transformer callback documented per CS-0090.1 and CS-0090.2.
   */
  Transformer m_transform;
  /**
   * @brief Pre-encode reverse transformation converting domain values into raw NamedObjects.
   * @details Invoked by encode() and getEncodedBitSize() prior to m_rawCodec delegation.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @compliance Private reverse-transformer callback documented per CS-0090.1 and CS-0090.2.
   */
  ReverseTransformer m_reverseTransform;
};

} // namespace datacodec

#endif // DATACODEC_ADVANCED_SCHEMA_HPP
