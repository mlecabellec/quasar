/**
 * @file ObjectDictionary.hpp
 * @brief Thread-safe CoE Object Dictionary storage and metadata descriptors.
 * @feature [TSK-20261113-001] Type-safe replacement for SOES SDOobjects[] globals
 */

#pragma once

#include "resoes/ResoesConstants.hpp"
#include "resoes/common.hpp"
#include <cstdint>
#include <map>
#include <mutex>
#include <span>
#include <string>
#include <vector>

namespace resoes {

/**
 * @brief CANopen / ETG.1000 Object Type codes (`OTYPE_*`).
 * @feature [TSK-20261113-001]
 */
enum class OdObjectType : uint16_t {
  Var = 0x0007U,    ///< Single variable (`max_sub = 0`)
  Array = 0x0008U,  ///< Homogeneous array
  Record = 0x0009U  ///< Heterogeneous record
};

/**
 * @brief CANopen / ETG.1000 Data Type codes (`DTYPE_*`).
 * @feature [TSK-20261113-001]
 */
enum class OdDataType : uint16_t {
  Boolean = 0x0001U,
  Integer8 = 0x0002U,
  Integer16 = 0x0003U,
  Integer32 = 0x0004U,
  Unsigned8 = 0x0005U,
  Unsigned16 = 0x0006U,
  Unsigned32 = 0x0007U,
  Real32 = 0x0008U,
  VisibleString = 0x0009U,
  OctetString = 0x000AU,
  Unsigned64 = 0x001BU,
  Integer64 = 0x0015U
};

/**
 * @brief CoE Object Entry access flags (`ATYPE_*`) matching ETG.1000.6 SDO Info.
 * @feature [TSK-20261113-001]
 */
namespace od_access {
constexpr uint16_t READ_PREOP = 0x0001U;  ///< Readable in Pre-Op
constexpr uint16_t READ_SAFEOP = 0x0002U; ///< Readable in Safe-Op
constexpr uint16_t READ_OP = 0x0004U;     ///< Readable in Op
constexpr uint16_t WRITE_PREOP = 0x0008U; ///< Writable in Pre-Op
constexpr uint16_t WRITE_SAFEOP = 0x0010U;///< Writable in Safe-Op
constexpr uint16_t WRITE_OP = 0x0020U;    ///< Writable in Op
constexpr uint16_t RX_PDO_MAP = 0x0040U;  ///< Mappable to RxPDO
constexpr uint16_t TX_PDO_MAP = 0x0080U;  ///< Mappable to TxPDO

constexpr uint16_t READ_ALL = READ_PREOP | READ_SAFEOP | READ_OP;
constexpr uint16_t WRITE_ALL = WRITE_PREOP | WRITE_SAFEOP | WRITE_OP;
constexpr uint16_t READ_WRITE_ALL = READ_ALL | WRITE_ALL;
} // namespace od_access

/**
 * @brief Single subindex descriptor and raw little-endian value storage.
 * @feature [TSK-20261113-001]
 */
struct OdSubindexEntry {
  uint8_t subindex = 0U;
  OdDataType data_type = OdDataType::Unsigned32;
  uint16_t bit_length = 32U;
  uint16_t access_flags = od_access::READ_ALL;
  std::string name{};
  std::vector<byte> data{};
};

/**
 * @brief Complete CoE Object Dictionary entry owning its subindex descriptors.
 * @feature [TSK-20261113-001]
 */
struct OdObjectEntry {
  uint16_t index = 0U;
  OdObjectType object_type = OdObjectType::Var;
  uint8_t max_subindex = 0U;
  std::string name{};
  std::vector<OdSubindexEntry> subindices{};
};

/**
 * @brief Thread-safe, instance-owned CoE Object Dictionary database.
 * @feature [TSK-20261113-001] [CS-0010.46]
 */
class ObjectDictionary final {
public:
  /// @brief Construct an empty Object Dictionary.
  ObjectDictionary() = default;

  /// @brief Default noexcept destructor [CS-0060.5].
  ~ObjectDictionary() noexcept = default;

  /**
   * @brief Insert or replace an object entry cleanly by index.
   * @param entry Complete OdObjectEntry descriptor (copied/moved in).
   * @return Success or SlvError::InvalidParameter.
   */
  [[nodiscard]] Result<> register_object(OdObjectEntry entry);

  /**
   * @brief Helper to add a simple single-value scalar variable (`OdObjectType::Var`).
   * @param index Object index (e.g. `0x1018`, `0x6000`).
   * @param data_type CoE data type.
   * @param bit_length Bit length (`1`..`64`).
   * @param access Access mask (`od_access::*`).
   * @param name Human-readable object name.
   * @param initial_bytes Initial little-endian byte payload.
   * @return Success or SlvError.
   */
  [[nodiscard]] Result<> add_variable(uint16_t index, OdDataType data_type,
                                      uint16_t bit_length, uint16_t access,
                                      const std::string &name,
                                      std::span<const byte> initial_bytes);

  /// @brief Return ordered snapshot of all registered object indices.
  [[nodiscard]] std::vector<uint16_t> list_indices() const;

  /// @brief Return copy of an OdObjectEntry descriptor or SlvError::ObjectNotFound.
  [[nodiscard]] Result<OdObjectEntry> get_object(uint16_t index) const;

  /// @brief Return copy of a single subindex entry or SlvError::ObjectNotFound.
  [[nodiscard]] Result<OdSubindexEntry> get_subindex(uint16_t index,
                                                     uint8_t subindex) const;

  /**
   * @brief Read raw little-endian bytes of a subindex entry.
   * @param index Object index.
   * @param subindex Subindex (`0`..`max_subindex`).
   * @return Vector of bytes or SlvError::ObjectNotFound.
   */
  [[nodiscard]] Result<std::vector<byte>> read_bytes(uint16_t index,
                                                     uint8_t subindex) const;

  /**
   * @brief Write raw little-endian bytes to an existing subindex entry.
   * @param index Object index.
   * @param subindex Subindex.
   * @param data Source bytes matching stored byte length (or shorter for strings).
   * @return Success or SlvError.
   */
  [[nodiscard]] Result<> write_bytes(uint16_t index, uint8_t subindex,
                                     std::span<const byte> data);

  /**
   * @brief Read typed scalar unsigned value up to 32 bits from Object Dictionary.
   * @param index Object index.
   * @param subindex Subindex.
   * @return Decoded uint32_t zero-extended or SlvError.
   */
  [[nodiscard]] Result<uint32_t> read_u32(uint16_t index,
                                          uint8_t subindex) const;

  /**
   * @brief Write typed scalar unsigned value (1/2/4 bytes depending on subindex size).
   * @param index Object index.
   * @param subindex Subindex.
   * @param value Scalar integer value.
   * @return Success or SlvError.
   */
  [[nodiscard]] Result<> write_u32(uint16_t index, uint8_t subindex,
                                   uint32_t value);

private:
  /// @brief Ordered index-keyed Object Dictionary storage.
  std::map<uint16_t, OdObjectEntry> objects_{};
  /// @brief Timed mutex serializing dictionary reads and writes [CS-0010.46].
  mutable std::timed_mutex mutex_{};
};

} // namespace resoes
