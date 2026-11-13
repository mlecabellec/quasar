/**
 * @file CoeServer.hpp
 * @brief CoE (CANopen over EtherCAT) SDO, SDO Info, Emergency & PDO mapping engine.
 * @feature [TSK-20261113-001] Symmetrical to SOES esc_coe.h/.c
 */

#pragma once

#include "resoes/EscController.hpp"
#include "resoes/EscTypes.hpp"
#include "resoes/MailboxServer.hpp"
#include "resoes/ObjectDictionary.hpp"
#include <cstdint>
#include <functional>
#include <mutex>
#include <vector>

namespace resoes {

/**
 * @brief Hook invoked before/after SDO writes for parameter validation/side effects.
 * @feature [TSK-20261113-001]
 */
class ICoeObserver {
public:
  /// @brief Virtual destructor ensuring safe polymorphic cleanup [CS-0060.5].
  virtual ~ICoeObserver() noexcept = default;

  /// @brief Called prior to committing an SDO write; return non-zero abort code to reject.
  [[nodiscard]] virtual uint32_t on_pre_sdo_write(
      uint16_t index, uint8_t subindex, std::span<const byte> new_data) = 0;

  /// @brief Called immediately after committing an SDO write.
  virtual void on_post_sdo_write(uint16_t index, uint8_t subindex) = 0;
};

/**
 * @brief CANopen over EtherCAT server implementing SDO, SDO Info, and dynamic PDO mapping.
 * @feature [TSK-20261113-001] [CS-0010.46]
 */
class CoeServer final : public IMailboxProtocolHandler {
public:
  /**
   * @brief Construct CoeServer bound to ObjectDictionary and EscController.
   * @param od Object Dictionary database.
   * @param controller Parent EscController for state/permission checks.
   */
  CoeServer(ObjectDictionary &od, EscController &controller);

  /// @brief Default noexcept destructor [CS-0060.5].
  ~CoeServer() noexcept override = default;

  /// @brief Register optional SDO download observer hook.
  void set_observer(ICoeObserver *observer);

  /// @brief Return `MBX_TYPE_COE`.
  [[nodiscard]] uint8_t protocol_type() const noexcept override;

  /// @brief Handle incoming CoE request (`SDOREQ`, `SDOINFO`) and produce reply.
  [[nodiscard]] uint16_t handle_request(const MailboxHeader &req_header,
                                        std::span<const byte> req_payload,
                                        std::vector<byte> &out_tx_payload,
                                        uint8_t &out_tx_type,
                                        bool &out_send_reply) override;

  /// @brief Clear any active segmented SDO download/upload session state.
  void reset() noexcept override;

  /**
   * @brief Compute dynamic mapped PDO size in bits for `0x1C12` (Rx) or `0x1C13` (Tx).
   * @param sm_assign_index `OD_IDX_SM2_ASSIGN` (`0x1C12`) or `OD_IDX_SM3_ASSIGN` (`0x1C13`).
   * @return Total bit length of mapped objects, or `0xFFFFFFFFU` / SlvError on invalid map.
   */
  [[nodiscard]] Result<uint32_t> compute_pdo_size_bits(
      uint16_t sm_assign_index) const;

  /**
   * @brief Compute both RxPDO (`SM2`) and TxPDO (`SM3`) byte sizes cleanly.
   * @param out_rx_bytes Computed SM2 byte length (rounded up).
   * @param out_tx_bytes Computed SM3 byte length (rounded up).
   * @return Success or SlvError.
   */
  [[nodiscard]] Result<> compute_pdo_sizes_bytes(uint16_t &out_rx_bytes,
                                                 uint16_t &out_tx_bytes) const;

  /**
   * @brief Pack all active TxPDO (`0x1C13`) OD values bit-by-bit into SM3 buffer.
   * @param tx_buffer Destination span sized >= computed TxPDO bytes.
   * @return Number of bits packed, or SlvError.
   */
  [[nodiscard]] Result<uint32_t> pack_tx_pdo(std::span<byte> tx_buffer) const;

  /**
   * @brief Unpack incoming SM2 RxPDO bytes bit-by-bit into mapped `0x1C12` OD entries.
   * @param rx_buffer Source span containing SM2 output process data.
   * @return Number of bits unpacked, or SlvError.
   */
  [[nodiscard]] Result<uint32_t> unpack_rx_pdo(
      std::span<const byte> rx_buffer);

  /**
   * @brief Build a CoE Emergency payload and enqueue it onto MailboxServer.
   * @param mailbox Mailbox server to receive Emergency frame.
   * @param error_code Standard Emergency Error Code (2 bytes).
   * @param error_register Error register value (`0x1001`).
   * @param manufacturer_data Up to 5 bytes of manufacturer diagnostic data.
   * @return Success or SlvError.
   */
  [[nodiscard]] Result<> emit_emergency(
      MailboxServer &mailbox, uint16_t error_code, uint8_t error_register,
      std::span<const byte> manufacturer_data);

private:
  enum class SegmentedMode : uint8_t { None = 0, Upload = 1, Download = 2 };

  struct SegmentedSession {
    SegmentedMode mode = SegmentedMode::None;
    uint16_t index = 0U;
    uint8_t subindex = 0U;
    bool complete_access = false;
    bool expected_toggle = false;
    uint32_t total_bytes = 0U;
    size_t offset = 0U;
    std::vector<byte> buffer{};
  };

  /// @brief Reference to Object Dictionary.
  ObjectDictionary &od_;
  /// @brief Reference to ESC state controller.
  EscController &controller_;
  /// @brief Optional SDO write validation/notification hook.
  ICoeObserver *observer_ = nullptr;
  /// @brief Active segmented SDO transfer state.
  SegmentedSession seg_{};
  /// @brief Timed mutex guarding segmented session state [CS-0010.46].
  mutable std::timed_mutex mutex_{};

  /// @brief Dispatch SDO request (`0x02`) into upload/download/segment/abort handlers.
  void handle_sdo_request_locked(std::span<const byte> req_payload,
                                 uint16_t max_mbx_payload,
                                 std::vector<byte> &out_tx_payload);

  /// @brief Dispatch SDO Information request (`0x08`) for ODList/OD/ED.
  void handle_sdo_info_locked(std::span<const byte> req_payload,
                              std::vector<byte> &out_tx_payload);

  /// @brief Handle Initiate SDO Upload (expedited, normal/segmented, or CA).
  void handle_upload_init_locked(const SdoHeader &hdr, uint16_t max_mbx_payload,
                                 std::vector<byte> &out_tx_payload);

  /// @brief Handle Segmented SDO Upload Request with toggle verification.
  void handle_upload_seg_locked(const SdoHeader &hdr, uint16_t max_mbx_payload,
                                std::vector<byte> &out_tx_payload);

  /// @brief Handle Initiate SDO Download (expedited, normal/segmented, or CA).
  void handle_download_init_locked(const SdoHeader &hdr,
                                   std::span<const byte> req_payload,
                                   std::vector<byte> &out_tx_payload);

  /// @brief Handle Segmented SDO Download Request and commit on last segment.
  void handle_download_seg_locked(const SdoHeader &hdr,
                                  std::span<const byte> req_payload,
                                  std::vector<byte> &out_tx_payload);

  /// @brief Construct a standard CoE SDO Abort response frame.
  static void build_sdo_abort(uint16_t index, uint8_t subindex,
                              uint32_t abort_code,
                              std::vector<byte> &out_tx_payload);

  /// @brief Assemble raw little-endian payload for SDO upload (single or Complete Access).
  [[nodiscard]] uint32_t gather_upload_bytes(uint16_t index, uint8_t subindex,
                                             bool complete_access,
                                             std::vector<byte> &out_data) const;

  /// @brief Validate permissions & commit downloaded bytes (single or Complete Access).
  [[nodiscard]] uint32_t commit_download_bytes(uint16_t index, uint8_t subindex,
                                               bool complete_access,
                                               std::span<const byte> payload);

  /// @brief Check whether current AL state allows read/write access under `access_flags`.
  [[nodiscard]] bool is_access_allowed(uint16_t access_flags,
                                       bool is_write) const;
};

} // namespace resoes
