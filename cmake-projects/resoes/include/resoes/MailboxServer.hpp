/**
 * @file MailboxServer.hpp
 * @brief Pooled ETG.1000 Mailbox buffer state machine and protocol multiplexer (`xOE`).
 * @feature [TSK-20261113-001] Symmetrical to SOES MBXcontrol & ESC_mbxprocess
 */

#pragma once

#include "resoes/EscController.hpp"
#include "resoes/EscTypes.hpp"
#include "resoes/IEscHardware.hpp"
#include "resoes/ResoesConstants.hpp"
#include <cstdint>
#include <mutex>
#include <span>
#include <vector>

namespace resoes {

/**
 * @brief Mailbox buffer slot states mirroring ETG.1000 / SOES `_MBXcontrol`.
 * @feature [TSK-20261113-001]
 */
enum class MailboxSlotState : uint8_t {
  Idle = 0,       ///< Slot available for TX allocation
  OwnerEcat,      ///< Active frame sitting in SM1 awaiting master read
  OwnerPdi,       ///< Slot claimed by SubDevice protocol handler
  XoeInProgress,  ///< Multi-step protocol exchange owns slot
  RepeatReq,      ///< Buffered in repeat-request slot for SM1 re-TX
  Again           ///< Waiting for SM1 availability to transmit
};

/**
 * @brief Mailbox protocol handler interface (CoE, FoE, EoE, SoE).
 * @feature [TSK-20261113-001]
 */
class IMailboxProtocolHandler {
public:
  /// @brief Virtual destructor ensuring safe polymorphic cleanup [CS-0060.5].
  virtual ~IMailboxProtocolHandler() noexcept = default;

  /// @brief Return ETG.1000 mailbox protocol type constant (`MBX_TYPE_COE`, etc.).
  [[nodiscard]] virtual uint8_t protocol_type() const noexcept = 0;

  /**
   * @brief Handle an incoming SM0 request payload and optionally emit a response.
   * @param req_header Parsed 6-byte ETG.1000 MailboxHeader.
   * @param req_payload Payload slice following the 6-byte header.
   * @param out_tx_payload Output buffer populated on response (protocol payload only).
   * @return Mailbox error code (`0` = ok, non-zero sends `MBXERR_*`), and response flag via out_send_reply.
   */
  [[nodiscard]] virtual uint16_t handle_request(
      const MailboxHeader &req_header, std::span<const byte> req_payload,
      std::vector<byte> &out_tx_payload, uint8_t &out_tx_type,
      bool &out_send_reply) = 0;

  /// @brief Reset multi-step protocol transfer state (e.g., on transition to INIT).
  virtual void reset() noexcept = 0;
};

/**
 * @brief Instance-owned ETG.1000 Mailbox Pool & SM0/SM1 state engine.
 * @feature [TSK-20261113-001] [CS-0010.46]
 */
class MailboxServer final {
public:
  /**
   * @brief Construct MailboxServer bound to ESC hardware and EscController.
   * @param hardware Non-null ESC hardware HAL.
   * @param controller Non-null EscController tracking SM0/SM1 configuration.
   */
  MailboxServer(IEscHardware &hardware, EscController &controller);

  /// @brief Default noexcept destructor [CS-0060.5].
  ~MailboxServer() noexcept = default;

  /// @brief Register a protocol handler (pointer must outlive MailboxServer).
  [[nodiscard]] Result<> register_handler(IMailboxProtocolHandler *handler);

  /// @brief Reset mailbox pool slots and pending counters cleanly.
  void reset();

  /**
   * @brief Service SM1 read/repeat completion and SM0 incoming requests.
   * @return Number of incoming/outgoing frames serviced, or SlvError.
   */
  [[nodiscard]] Result<size_t> process();

  /**
   * @brief Queue an unsolicited/async outgoing mailbox payload (e.g., CoE Emergency).
   * @param protocol_type ETG.1000 mailbox type (`MBX_TYPE_*`).
   * @param payload Protocol bytes excluding 6-byte MailboxHeader.
   * @return Success or SlvError::MailboxFull / SlvError::InvalidState.
   */
  [[nodiscard]] Result<> enqueue_tx(uint8_t protocol_type,
                                    std::span<const byte> payload);

  /// @brief Query count of active pending/queued TX slots.
  [[nodiscard]] size_t pending_tx_count() const;

private:
  struct BufferSlot {
    MailboxSlotState state = MailboxSlotState::Idle;
    uint8_t mbx_type = 0U;
    uint16_t length = 0U;
    std::vector<byte> frame{};
  };

  /// @brief ESC hardware interface.
  IEscHardware &hardware_;
  /// @brief Parent EscController for SM0/SM1 activation & state checks.
  EscController &controller_;
  /// @brief Pooled mailbox TX/repeat slots (eliminates SOES global `MBX[]`).
  std::vector<BufferSlot> slots_{};
  /// @brief Registered protocol handlers (CoE, FoE, EoE, SoE).
  std::vector<IMailboxProtocolHandler *> handlers_{};
  /// @brief Last accepted SM0 mailbox counter (`1..7`, `0` unassigned).
  uint8_t last_rx_counter_ = 0U;
  /// @brief Outgoing mailbox counter (`1..7`).
  uint8_t next_tx_counter_ = 1U;
  /// @brief Timed mutex serializing mailbox pool access [CS-0010.46].
  mutable std::timed_mutex mutex_{};

  /// @brief Flush first `Again` slot into SM1 if hardware mailbox buffer is empty.
  [[nodiscard]] Result<bool> flush_tx_queue_locked();

  /// @brief Build and enqueue a standard 6B+4B ETG.1000 `MBX_error` packet.
  [[nodiscard]] Result<> queue_mbx_error_locked(uint16_t error_code);

  /// @brief Allocate next available `Idle` slot index or return `SIZE_MAX`.
  [[nodiscard]] size_t allocate_slot_locked() const noexcept;

  /// @brief Advance outgoing mailbox sequence counter cleanly (`1..7` wrap).
  [[nodiscard]] uint8_t consume_tx_counter_locked() noexcept;
};

} // namespace resoes
