/**
 * @file Schema.hpp
 * @brief Schema definitions for datacodec.
 * @details Defines FieldDef and ContainerDef nodes built atop quasar::named::NamedObject to model
 * structured binary layouts and hierarchical object schemas.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020, FE-0040
 * @compliance Enforces file-level Doxygen and traceability standards (CS-0090.1, CS-0090.2).
 */

#ifndef DATACODEC_SCHEMA_HPP
#define DATACODEC_SCHEMA_HPP

// Include codec interface, NamedObject base class, and standard container/pointer utilities.
#include "datacodec/ICodec.hpp"
#include "quasar/named/NamedObject.hpp"
// Standard functional, memory, string, and vector headers.
#include <functional>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Namespace for Quasar binary and schema-driven data codecs and mappers.
 * @details Houses FieldDef and ContainerDef schema modeling primitives.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Encloses schema declarations inside namespace datacodec (CS-0010.33).
 */
namespace datacodec {

/**
 * @class FieldDef
 * @brief Definition of a single field in the schema.
 * @details Inherits from quasar::named::NamedObject, fulfilling FE-0020.4 by providing
 * a concrete implementation of a named object for data fields. It associates
 * an ICodec with a specific bit offset within a larger structure.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020, FE-0040
 * @exposed Public schema node describing a typed field and its bit offset.
 * @compliance Documents class hierarchy role, factory/constructor contracts, and accessors (CS-0090.1..CS-0090.9).
 * @invariant m_codec and m_bitOffset remain immutable after construction.
 */
class FieldDef : public quasar::named::NamedObject {
public:
  /**
   * @brief Factory creating a shared FieldDef instance with name, codec, and bit offset.
   * @details Allocates a FieldDef via std::make_shared without manually binding weak self pointers.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public factory helper used when building ContainerDef schemas.
   * @compliance Documents parameter/return contracts, allocation failures, and containment (CS-0090.1..CS-0090.9).
   * @param[in] name
   *   Justification: Field identifier used as both NamedObject node name and JSON key name.
   *   Usage: Forwarded to FieldDef constructor and stored as the NamedObject name.
   *   Consumed: Copied into NamedObject internal storage; caller retains ownership of input string.
   *   Prerequisites: Valid non-empty field name satisfying NamedObject naming constraints.
   * @param[in] codec
   *   Justification: Codec defining wire/type serialization rules for this field.
   *   Usage: Retained as shared_ptr in m_codec.
   *   Consumed: Shared reference-count incremented via copy into FieldDef.
   *   Prerequisites: Non-null std::shared_ptr<ICodec> expected by mappers.
   * @param[in] bitOffset
   *   Justification: Starting bit offset of this field within its enclosing binary layout.
   *   Usage: Stored in m_bitOffset (defaults to 0).
   *   Consumed: Scalar value copied into FieldDef.
   *   Prerequisites: Non-negative bit offset appropriate for container frame size.
   * @return Shared pointer to the newly constructed FieldDef object.
   *   Justification: Enables shared ownership inside ContainerDef field collections and NamedObject trees.
   *   Usage: Passed to ContainerDef::addField() or stored directly by schema builders.
   *   Consumed: Ownership shared with caller via std::shared_ptr<FieldDef>.
   *   Prerequisites: Valid name satisfying NamedObject rules and available heap memory.
   *   Limitations: Does not invoke self-pointer registration explicitly if parent tree attachment relies on setParent.
   *   Lifecycle: Heap-allocated via std::make_shared; destroyed when last shared_ptr releases.
   *   Modification / Deletion: Caller may add to containers, inspect getters, or release ownership.
   *   Specific Values: Non-null std::shared_ptr<FieldDef> on success.
   *   Error Behavior: Propagates std::bad_alloc or NamedObject name-validation exceptions on failure.
   * @pre name satisfies NamedObject naming constraints and codec is non-null.
   * @post Returned FieldDef has getName() == name, getCodec() == codec, getBitOffset() == bitOffset.
   * @throws std::exception if NamedObject construction or shared allocation fails.
   * @containment
   *   Approach: Uses std::make_shared for exception-safe single-allocation object construction.
   *   Limitations: Does not reject null codec eagerly in factory; callers must supply non-null codec.
   */
  static std::shared_ptr<FieldDef> create(const std::string &name,
                                          std::shared_ptr<ICodec> codec,
                                          size_t bitOffset = 0) {
    std::shared_ptr<FieldDef> obj =
        std::make_shared<FieldDef>(name, codec, bitOffset);
    // Self-registration via setSelf is not invoked here because NamedObject parent attachment handles lifetime.
    return obj;
  }

  /**
   * @brief Constructs a FieldDef associating a field name with a codec and bit offset.
   * @details Initializes base NamedObject with name and binds codec and bitOffset members.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public constructor used by FieldDef::create and derived ConditionalFieldDef.
   * @compliance Documents constructor parameter contracts and preconditions (CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.9).
   * @param[in] name
   *   Justification: Identifier for the schema field within its parent container.
   *   Usage: Passed directly to quasar::named::NamedObject base constructor.
   *   Consumed: Copied into base NamedObject storage.
   *   Prerequisites: Valid NamedObject name string.
   * @param[in] codec
   *   Justification: Polymorphic codec handling value serialization/type reporting.
   *   Usage: Stored in m_codec.
   *   Consumed: Copied into m_codec (increments shared_ptr use count).
   *   Prerequisites: Non-null std::shared_ptr<ICodec>.
   * @param[in] bitOffset
   *   Justification: Bit offset of the field relative to container start.
   *   Usage: Stored in m_bitOffset.
   *   Consumed: Copied by value.
   *   Prerequisites: Non-negative bit index.
   * @pre name obeys NamedObject rules and codec is initialized.
   * @post Member accessors reflect passed name, codec, and bitOffset.
   */
  FieldDef(const std::string &name, std::shared_ptr<ICodec> codec,
           size_t bitOffset)
      : quasar::named::NamedObject(name), m_codec(codec),
        m_bitOffset(bitOffset) {}

  /**
   * @brief Returns the shared codec assigned to this schema field.
   * @details Provides shared ownership access to m_codec for decoding, encoding, or type inspection.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public accessor used by BinaryMapper and JsonMapper.
   * @compliance Documents return ownership contract and containment (CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9).
   * @return Shared pointer copy referencing m_codec.
   *   Justification: Allows mappers to invoke decode, encode, and getCodecType on the field's codec.
   *   Usage: Dereferenced by callers after ensuring non-null pointer.
   *   Consumed: Shared ownership copy; increments m_codec reference count.
   *   Prerequisites: FieldDef constructed validly.
   *   Limitations: Returns nullptr if FieldDef was constructed with a null codec pointer.
   *   Lifecycle: Valid as long as either FieldDef or caller shared_ptr copy remains alive.
   *   Modification / Deletion: Caller may release its shared_ptr; mutating target ICodec depends on constness.
   *   Specific Values: The std::shared_ptr<ICodec> passed at construction.
   *   Error Behavior: Cannot fail; shared_ptr copy constructor is noexcept in C++ standard library.
   * @post Returned shared_ptr aliases m_codec.
   * @throws None in practice; signature omits noexcept for ABI flexibility.
   * @containment
   *   Approach: Returns value copy of shared_ptr safely extending target lifetime.
   *   Limitations: Callers must check for nullptr if untrusted construction is possible.
   */
  std::shared_ptr<ICodec> getCodec() const { return m_codec; }

  /**
   * @brief Returns the configured bit offset of this field within its enclosing container.
   * @details Supplies the zero-based starting bit position used by BinaryMapper slicing.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public layout offset accessor.
   * @compliance Documents scalar return contract and containment (CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9).
   * @return Bit offset relative to container buffer start.
   *   Justification: Determines BitBufferSlice start index during binary field extraction.
   *   Usage: Passed as offset argument to BitBufferSlice::slice().
   *   Consumed: Plain scalar copy; no ownership transfer.
   *   Prerequisites: FieldDef constructed validly.
   *   Limitations: Static offset only; does not dynamically account for prior variable-length fields.
   *   Lifecycle: Scalar size_t scoped to caller expression.
   *   Modification / Deletion: Caller owns scalar copy.
   *   Specific Values: Value of m_bitOffset assigned at construction.
   *   Error Behavior: Cannot fail; direct scalar member read.
   * @post Return value equals m_bitOffset.
   * @throws None in practice; direct member read.
   * @containment
   *   Approach: Read-only scalar getter without mutation or allocations.
   *   Limitations: Does not validate whether offset fits within a specific container buffer.
   */
  size_t getBitOffset() const { return m_bitOffset; }

private:
  /**
   * @brief Codec instance responsible for serializing and typing this field's data.
   * @details Held via shared pointer so multiple fields can share stateless codec instances.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @compliance Private member documented per CS-0090.1 and CS-0090.2.
   */
  std::shared_ptr<ICodec> m_codec;
  /**
   * @brief Starting bit offset of this field inside its parent binary container.
   * @details Measured in bits from the beginning of the enclosing BitBufferSlice.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @compliance Private member documented per CS-0090.1 and CS-0090.2.
   */
  size_t m_bitOffset;
};

/**
 * @class ContainerDef
 * @brief Definition of a container (composite structure) in the schema.
 * @details These definitions describe the structure of EtherCAT data objects,
 * such as those found in SDO (Service Data Object) entries, PDO (Process Data
 * Object) mappings, and the payloads of various mailbox protocols (CoE, FoE, EoE).
 * By defining these structures, the datacodec module enables the accurate
 * serialization and deserialization of data required for EtherCAT communication,
 * directly supporting features like FE-0040.4 (Mailbox Protocols) and
 * FE-0040.5 (Process Data Configuration).
 *
 * @par Contribution to FE-0020
 * Inherits from quasar::named::NamedObject (FE-0020.4) and manages a collection of
 * FieldDef objects. This establishes a hierarchical structure, directly contributing
 * to FE-0020.1.2 and FE-0020.1.3 by enabling the construction of trees or graphs of
 * named objects, where ContainerDef acts as a parent to FieldDef instances.
 *
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020, FE-0040
 * @exposed Public composite schema node used by BinaryMapper and JsonMapper.
 * @compliance Documents container hierarchy contracts, invariants, and accessors (CS-0090.1..CS-0090.9).
 * @invariant Ordered field vector m_fields preserves registration sequence of added FieldDef nodes.
 */
class ContainerDef : public quasar::named::NamedObject {
public:
  /**
   * @brief Factory creating a shared ContainerDef instance with the specified name.
   * @details Allocates a ContainerDef wrapped in std::shared_ptr for schema assembly.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public factory helper for composite schema creation.
   * @compliance Documents name parameter, return contract, and containment (CS-0090.1..CS-0090.9).
   * @param[in] name
   *   Justification: Container identifier propagated as root NamedObject node name by mappers.
   *   Usage: Forwarded to ContainerDef constructor.
   *   Consumed: Copied into base NamedObject storage.
   *   Prerequisites: Valid non-empty name matching NamedObject naming rules.
   * @return Shared pointer to the newly created ContainerDef instance.
   *   Justification: Facilitates shared schema reuse across multiple encode/decode calls.
   *   Usage: Populated via addField() and passed to BinaryMapper or JsonMapper.
   *   Consumed: Ownership transferred/shared with caller via std::shared_ptr.
   *   Prerequisites: Valid name and available heap memory.
   *   Limitations: Initially contains zero fields until addField() is called.
   *   Lifecycle: Heap-allocated via std::make_shared; freed when all shared_ptrs drop.
   *   Modification / Deletion: Caller may append fields via addField() and release when done.
   *   Specific Values: Non-null std::shared_ptr<ContainerDef> on success.
   *   Error Behavior: Propagates allocation or NamedObject name-validation exceptions.
   * @pre name satisfies NamedObject naming requirements.
   * @post Returned ContainerDef has getName() == name and empty getFields().
   * @throws std::exception if allocation or base NamedObject construction fails.
   * @containment
   *   Approach: Single-allocation std::make_shared construction.
   *   Limitations: Single-threaded setup expected during initial field registration.
   */
  static std::shared_ptr<ContainerDef> create(const std::string &name) {
    std::shared_ptr<ContainerDef> obj = std::make_shared<ContainerDef>(name);
    return obj;
  }

  /**
   * @brief Constructs a named ContainerDef with an empty field collection.
   * @details Delegates container naming to the quasar::named::NamedObject base class.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public constructor used directly or via ContainerDef::create().
   * @compliance Documents constructor contract and postcondition (CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.9).
   * @param[in] name
   *   Justification: Schema container name.
   *   Usage: Forwarded to quasar::named::NamedObject base constructor.
   *   Consumed: Copied into base NamedObject name storage.
   *   Prerequisites: Valid NamedObject identifier string.
   * @pre name satisfies NamedObject validation rules.
   * @post getName() == name and getFields().empty() == true.
   */
  ContainerDef(const std::string &name) : quasar::named::NamedObject(name) {}

  /**
   * @brief Appends a field definition to this container's ordered field list.
   * @details Pushes field onto m_fields while keeping field hierarchy tracking separate from
   * NamedObject parent/child tree attachment.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100, FE-0020
   * @exposed Public schema builder method for adding fields to a container.
   * @compliance Documents parameter contract, mutation, and containment (CS-0090.1..CS-0090.9).
   * @param[in] field
   *   Justification: Field definition to register in this container schema.
   *   Usage: Appended to m_fields via push_back.
   *   Consumed: Shared reference count incremented inside m_fields; caller retains shared access.
   *   Prerequisites: Non-null std::shared_ptr<FieldDef> expected for safe mapper dereferencing.
   * @pre field is a valid FieldDef pointer (non-null for valid schema operation).
   * @post getFields().back() == field when no exception occurs.
   * @throws std::bad_alloc if internal std::vector reallocation fails.
   * @containment
   *   Approach: Relies on std::vector::push_back strong exception safety guarantee.
   *   Limitations: Does not deduplicate field names or validate non-null pointers eagerly;
   *   also stores fields in m_fields only without attaching them as NamedObject children.
   */
  void addField(std::shared_ptr<FieldDef> field) {
    m_fields.push_back(field);
    // Fields are stored in m_fields order without also registering them as NamedObject children here.
  }

  /**
   * @brief Returns an immutable reference to the ordered list of registered FieldDef objects.
   * @details Exposes m_fields without copying so mappers can iterate fields in schema order.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @exposed Public schema field-list accessor used by BinaryMapper and JsonMapper.
   * @compliance Documents lifetime, aliasing rules, and error containment (CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9).
   * @return Const reference to internal vector of std::shared_ptr<FieldDef>.
   *   Justification: Enables zero-copy iteration over container fields during encode/decode.
   *   Usage: Iterated via range-based for loops by mappers and schema inspectors.
   *   Consumed: Read-only reference to internal vector; no ownership transferred.
   *   Prerequisites: ContainerDef remains alive and unmutated (no concurrent addField()) during reference use.
   *   Limitations: Vector reallocation from subsequent addField() invalidates returned reference and iterators.
   *   Lifecycle: Reference remains valid until ContainerDef destruction or vector reallocation.
   *   Modification / Deletion: Caller cannot modify vector through const reference and must not delete it.
   *   Specific Values: Ordered sequence of FieldDef pointers registered via addField().
   *   Error Behavior: Cannot fail; returns reference to member vector directly.
   * @post Returned reference aliases m_fields of *this.
   * @throws None in practice; direct const reference return.
   * @containment
   *   Approach: Const reference prevents caller mutation of field ordering.
   *   Limitations: Not thread-safe against concurrent addField() modifications.
   */
  const std::vector<std::shared_ptr<FieldDef>> &getFields() const {
    return m_fields;
  }

private:
  /**
   * @brief Ordered collection of fields comprising this composite container schema.
   * @details Preserves registration order for deterministic field parsing and serialization.
   * @task TSK-20261113-003, TSK-20260303-002
   * @feature FE-0010, FE-0100
   * @compliance Private member container documented per CS-0090.1 and CS-0090.2.
   */
  std::vector<std::shared_ptr<FieldDef>> m_fields;
};

} // namespace datacodec

#endif // DATACODEC_SCHEMA_HPP
