// vim:ts=4 expandtab: 
/******************************************************************************
 *                                                                            *
 * File:        plx_regs.h                                                    *
 *                                                                            *
 * Description: The interface to the AI64 Linux device driver.                *
 *                                                                            *
 * Date:        07/15/2004                                                    *
 * History:                                                                   *
 *                                                                            *
 *   2  07/15/04   R. Calabro                                                 *
 *      Updated to run on RedHawk 1.4                                         *
 *                                                                            *
 *   1  Nov 2002   Evan Hillman                                               *
 *      This is the header used by AI64 driver.                               *
 *                                                                            *
 ******************************************************************************/

#ifndef PLX_REGS
#define PLX_REGS

//** DMA Registers **
#ifndef AI64_DMACH_0_MODE
#define AI64_DMACH_0_MODE				(0x80/4)
#endif
#ifndef AI64_AI64_DMACH_0_PCI_ADDR
#define AI64_DMACH_0_PCI_ADDR			(0x84/4)
#endif
#ifndef AI64_DMACH_0_LOCAL_ADDR
#define AI64_DMACH_0_LOCAL_ADDR			(0x88/4)
#endif
#ifndef AI64_DMACH_0_TRANS_BYTE_CNT
#define AI64_DMACH_0_TRANS_BYTE_CNT		(0x8C/4)
#endif
#ifndef AI64_DMACH_0_DESC_PTR
#define AI64_DMACH_0_DESC_PTR			(0x90/4)
#endif

#ifndef AI64_DMACH_1_MODE
#define AI64_DMACH_1_MODE				(0x94/4)
#endif
#ifndef AI64_DMACH_1_PCI_ADDR
#define AI64_DMACH_1_PCI_ADDR			(0x98/4)
#endif
#ifndef AI64_DMACH_1_LOCAL_ADDR
#define AI64_DMACH_1_LOCAL_ADDR			(0x9C/4)
#endif
#ifndef AI64_DMACH_1_TRANS_BYTE_CNT
#define AI64_DMACH_1_TRANS_BYTE_CNT		(0xA0/4)
#endif
#ifndef AI64_DMACH_1_DESC_PTR
#define AI64_DMACH_1_DESC_PTR			(0xA4/4)
#endif

#ifndef AI64_DMACMD_STATUS
#define AI64_DMACMD_STATUS				(0xA8/4)
#endif
#ifndef AI64_DMAMODE_ARBITRATION
#define AI64_DMAMODE_ARBITRATION		(0xAC/4)
#endif

#ifndef AI64_DMATHRESHOLD_REG
#define AI64_DMATHRESHOLD_REG			(0xB0/4)
#endif

//** Local Configuration Registers. **
#ifndef PCI_TO_LOC_ADDR_0_RNG
#define PCI_TO_LOC_ADDR_0_RNG		(0x00/4)
#endif
#ifndef LOC_BASE_ADDR_REMAP_0
#define LOC_BASE_ADDR_REMAP_0		(0x04/4)
#endif
#ifndef MODE_ARBITRATION
#define MODE_ARBITRATION			(0x08/4)
#endif
#ifndef BIG_LITTLE_ENDIAN_DESC
#define BIG_LITTLE_ENDIAN_DESC		(0x0C/4)
#endif
#ifndef PCI_TO_LOC_ROM_RNG
#define PCI_TO_LOC_ROM_RNG			(0x10/4)
#endif
#ifndef LOC_BASE_ADDR_REMAP_EXP_ROM
#define LOC_BASE_ADDR_REMAP_EXP_ROM	(0x14/4)
#endif
#ifndef BUS_REG_DESC_0_FOR_PCI_LOC
#define BUS_REG_DESC_0_FOR_PCI_LOC	(0x18/4)
#endif
#ifndef DIR_MASTER_TO_PCI_RNG
#define DIR_MASTER_TO_PCI_RNG		(0x1C/4)
#endif
#ifndef LOC_ADDR_FOR_DIR_MASTER_MEM
#define LOC_ADDR_FOR_DIR_MASTER_MEM	(0x20/4)
#endif
#ifndef LOC_ADDR_FOR_DIR_MASTER_IO
#define LOC_ADDR_FOR_DIR_MASTER_IO	(0x24/4)
#endif
#ifndef PCI_ADDR_REMAP_DIR_MASTER
#define PCI_ADDR_REMAP_DIR_MASTER	(0x28/4)
#endif
#ifndef PCI_CFG_ADDR_DIR_MASTER_IO
#define PCI_CFG_ADDR_DIR_MASTER_IO	(0x2C/4)
#endif
 

#ifndef PCI_TO_LOC_ADDR_1_RNG
#define PCI_TO_LOC_ADDR_1_RNG		(0xF0/4)
#endif
#ifndef LOC_BASE_ADDR_REMAP_1
#define LOC_BASE_ADDR_REMAP_1		(0xF4/4)
#endif
#ifndef BUS_REG_DESC_1_FOR_PCI_LOC
#define BUS_REG_DESC_1_FOR_PCI_LOC	(0xF8/4)
#endif
 

//** Run Time Registers **
#ifndef MAILBOX_REGISTER_0
#define MAILBOX_REGISTER_0			(0x40/4)
#endif
#ifndef MAILBOX_REGISTER_1
#define MAILBOX_REGISTER_1			(0x44/4)
#endif
#ifndef MAILBOX_REGISTER_2
#define MAILBOX_REGISTER_2			(0x48/4)
#endif
#ifndef MAILBOX_REGISTER_3
#define MAILBOX_REGISTER_3			(0x4C/4)
#endif
#ifndef MAILBOX_REGISTER_4
#define MAILBOX_REGISTER_4			(0x50/4)
#endif
#ifndef MAILBOX_REGISTER_5
#define MAILBOX_REGISTER_5			(0x54/4)
#endif
#ifndef MAILBOX_REGISTER_6
#define MAILBOX_REGISTER_6			(0x58/4)
#endif
#ifndef MAILBOX_REGISTER_7
#define MAILBOX_REGISTER_7			(0x5C/4)
#endif
#ifndef PCI_TO_LOC_DOORBELL
#define PCI_TO_LOC_DOORBELL			(0x60/4)
#endif
#ifndef LOC_TO_PCI_DOORBELL
#define LOC_TO_PCI_DOORBELL			(0x64/4)
#endif
#ifndef INT_CTRL_STATUS
#define INT_CTRL_STATUS				(0x68/4)
#endif
#ifndef PROM_CTRL_CMD_CODES_CTRL
#define PROM_CTRL_CMD_CODES_CTRL	(0x6C/4)
#endif
#ifndef DEVICE_ID_VENDOR_ID
#define DEVICE_ID_VENDOR_ID			(0x70/4)
#endif
#ifndef REVISION_ID
#define REVISION_ID					(0x74/4)
#endif
#ifndef MAILBOX_REG_0
#define MAILBOX_REG_0				(0x78/4)
#endif
#ifndef MAILBOX_REG_1
#define MAILBOX_REG_1				(0x7C/4)
#endif

//** Messaging Queue Registers **
#ifndef OUT_POST_Q_INT_STATUS
#define OUT_POST_Q_INT_STATUS        (0x30/4)
#endif
#ifndef OUT_POST_Q_INT_MASK
#define OUT_POST_Q_INT_MASK          (0x34/4)
#endif
#ifndef IN_Q_PORT
#define IN_Q_PORT                    (0x40/4)
#endif
#ifndef OUT_Q_PORT
#define OUT_Q_PORT                   (0x44/4)
#endif

#ifndef MSG_UNIT_CONFIG
#define MSG_UNIT_CONFIG              (0xC0/4)
#endif
#ifndef Q_BASE_ADDR
#define Q_BASE_ADDR                  (0xC4/4)
#endif
#ifndef IN_FREE_HEAD_PTR
#define IN_FREE_HEAD_PTR             (0xC8/4)
#endif
#ifndef IN_FREE_TAIL_PTR
#define IN_FREE_TAIL_PTR             (0xCC/4)
#endif
#ifndef IN_POST_HEAD_PTR
#define IN_POST_HEAD_PTR             (0xD0/4)
#endif
#ifndef IN_POST_TAIL_PTR
#define IN_POST_TAIL_PTR             (0xD4/4)
#endif
#ifndef OUT_FREE_HEAD_PTR
#define OUT_FREE_HEAD_PTR            (0xD8/4)
#endif
#ifndef OUT_FREE_TAIL_PTR
#define OUT_FREE_TAIL_PTR            (0xDC/4)
#endif
#ifndef OUT_POST_HEAD_PTR
#define OUT_POST_HEAD_PTR            (0xE0/4)
#endif
#ifndef OUT_POST_TAIL_PTR
#define OUT_POST_TAIL_PTR            (0xE4/4)
#endif
#ifndef Q_STATUS_CTRL_REG
#define Q_STATUS_CTRL_REG            (0xE8/4)
#endif

// Interrupt Control/Status register masks

#ifndef IRQ_LOCAL_LSERR_ABORT
#define IRQ_LOCAL_LSERR_ABORT		(1 << 0)
#endif
#ifndef IRQ_LOCAL_LSERR_OVERFLOW
#define IRQ_LOCAL_LSERR_OVERFLOW	(1 << 1)
#endif
#ifndef IRQ_GENERATE_SERR
#define IRQ_GENERATE_SERR			(1 << 2)
#endif
#ifndef IRQ_MAILBOX_ENABLE
#define IRQ_MAILBOX_ENABLE			(1 << 3)
#endif
#ifndef IRQ_RESERVED1
#define IRQ_RESERVED1				(1 << 4)
#endif
#ifndef IRQ_RESERVED2
#define IRQ_RESERVED2				(1 << 5)
#endif
#ifndef IRQ_RESERVED3
#define IRQ_RESERVED3				(1 << 6)
#endif
#ifndef IRQ_RESERVED4
#define IRQ_RESERVED4				(1 << 7)
#endif
#ifndef IRQ_PCI_ENABLE
#define IRQ_PCI_ENABLE				(1 << 8)
#endif
#ifndef IRQ_PCI_DOORBELL_ENABLE
#define IRQ_PCI_DOORBELL_ENABLE		(1 << 9)
#endif
#ifndef IRQ_ABORT_ENABLE
#define IRQ_ABORT_ENABLE			(1 << 10)
#endif
#ifndef IRQ_LOCAL_PCI_ENABLE
#define IRQ_LOCAL_PCI_ENABLE		(1 << 11)
#endif
#ifndef IRQ_RETRY_ENABLE
#define IRQ_RETRY_ENABLE			(1 << 12)
#endif
#ifndef IRQ_PCI_DOORBELL_ACTIVE
#define IRQ_PCI_DOORBELL_ACTIVE		(1 << 13)
#endif
#ifndef IRQ_ABORT_ACTIVE
#define IRQ_ABORT_ACTIVE			(1 << 14)
#endif
#ifndef IRQ_LOCAL_ACTIVE
#define IRQ_LOCAL_ACTIVE			(1 << 15)
#endif
#ifndef IRQ_LOCAL_ENABLE
#define IRQ_LOCAL_ENABLE			(1 << 16)
#endif
#ifndef IRQ_LOCAL_DOORBELL_ENABLE
#define IRQ_LOCAL_DOORBELL_ENABLE	(1 << 17)
#endif
#ifndef IRQ_DMA_0_ENABLE
#define IRQ_DMA_0_ENABLE			(1 << 18)
#endif
#ifndef IRQ_DMA_1_ENABLE
#define IRQ_DMA_1_ENABLE			(1 << 19)
#endif
#ifndef IRQ_LOCAL_DOORBELL_ACTIVE
#define IRQ_LOCAL_DOORBELL_ACTIVE	(1 << 20)
#endif
#ifndef IRQ_DMA_0_ACTIVE
#define IRQ_DMA_0_ACTIVE			(1 << 21)
#endif
#ifndef IRQ_DMA_1_ACTIVE
#define IRQ_DMA_1_ACTIVE			(1 << 22)
#endif
#ifndef IRQ_BIST_ACTIVE
#define IRQ_BIST_ACTIVE				(1 << 23)
#endif
#ifndef IRQ_MASTER_ABORT
#define IRQ_MASTER_ABORT			(1 << 24)
#endif
#ifndef IRQ_DMA_0_ABORT
#define IRQ_DMA_0_ABORT				(1 << 25)
#endif
#ifndef IRQ_DMA_1_ABORT
#define IRQ_DMA_1_ABORT				(1 << 26)
#endif
#ifndef IRQ_TARGET_ABORT
#define IRQ_TARGET_ABORT			(1 << 27)
#endif
#ifndef IRQ_MAILBOX_0
#define IRQ_MAILBOX_0				(1 << 28)
#endif
#ifndef IRQ_MAILBOX_1
#define IRQ_MAILBOX_1				(1 << 29)
#endif
#ifndef IRQ_MAILBOX_2
#define IRQ_MAILBOX_2				(1 << 30)
#endif
#ifndef IRQ_MAILBOX_3
#define IRQ_MAILBOX_3				(1 << 31)
#endif

#ifndef NON_DEMAND_DMA_MODE
#define NON_DEMAND_DMA_MODE 0x00000840
#endif
#ifndef DEMAND_DMA_MODE
#define DEMAND_DMA_MODE 	0x00001840
#endif
 

#ifndef ENABLE_CHAINING
#define ENABLE_CHAINING (1 << 9)
#endif
#ifndef DMA_PCI_IRQ_SELECT
#define DMA_PCI_IRQ_SELECT (1 << 17)
#endif
 

// DMA Command/Status Register Masks. Channel 0 and 1
// share the same register, so care must be taken not to
// overwrite one while writing the other.

#ifndef DMA_PRESERVE_CHANNEL_MASK_0
#define DMA_PRESERVE_CHANNEL_MASK_0	0xff
#endif
#ifndef CH0_DMA_ENABLE_MASK
#define CH0_DMA_ENABLE_MASK		(1 << 0)
#endif
#ifndef CH0_DMA_START_MASK
#define CH0_DMA_START_MASK		(1 << 1)
#endif
#ifndef CH0_DMA_ABORT_MASK
#define CH0_DMA_ABORT_MASK		(1 << 2)
#endif
#ifndef CH0_DMA_CLEAR_IRQ_MASK
#define CH0_DMA_CLEAR_IRQ_MASK	(1 << 3)
#endif
#ifndef CH0_DMA_CLEAR_IRQ_MASK
#define CH0_DMA_CLEAR_IRQ_MASK		(1 << 4)
#endif

#ifndef DMA_PRESERVE_CHANNEL_MASK_1
#define DMA_PRESERVE_CHANNEL_MASK_1	(0xff << 8)
#endif
#ifndef CH1_DMA_ENABLE_MASK
#define CH1_DMA_ENABLE_MASK		(1 << 8)
#endif
#ifndef CH1_DMA_START_MASK
#define CH1_DMA_START_MASK		(1 << 9)
#endif
#ifndef CH1_DMA_ABORT_MASK
#define CH1_DMA_ABORT_MASK		(1 << 10)
#endif
#ifndef CH1_DMA_CLEAR_IRQ_MASK
#define CH1_DMA_CLEAR_IRQ_MASK	(1 << 11)
#endif
#ifndef CH1_DMA_DONE_MASK
#define CH1_DMA_DONE_MASK		(1 << 12)
#endif

//#define CH1_DMA_DONE_MASK 0x1000

// DMA Mode Register Masks. 
#ifndef DMA_MODE_DEFAULT
#define DMA_MODE_DEFAULT       0x00000D40
#endif
#ifndef DMA_MODE_CHAINING
#define DMA_MODE_CHAINING      0x00000200
#endif
#ifndef DMA_MODE_DEMAND_ENABLE
#define DMA_MODE_DEMAND_ENABLE 0x00001000
#endif


// DMA Descriptor Pointer Register Masks. 
#ifndef DPR_RW_SHIFT
#define DPR_RW_SHIFT            3
#endif
#ifndef DPR_INT_ON_DMA_COMPLETE
#define DPR_INT_ON_DMA_COMPLETE 0x0004
#endif
#ifndef DPR_END_OF_CHAIN
#define DPR_END_OF_CHAIN        0x0002
#endif

// DMA Command/Status Register Masks. 
#ifndef START_DMA_CMD
#define START_DMA_CMD      0x000B
#endif
#ifndef STOP_DMA_CMD_MASK
#define STOP_DMA_CMD_MASK  0xFF01
#endif
#ifndef ABORT_DMA_CMD_MASK
#define ABORT_DMA_CMD_MASK 0x0004
#endif

// DMA Command/Status Register Masks. Channel 0 and 1
// share the same register, so care must be taken not to
// overwrite one while writing the other.
#ifndef DMA_PRESERVE_CHANNEL_MASK_0
#define DMA_PRESERVE_CHANNEL_MASK_0	0xff
#endif
#ifndef CH0_DMA_ENABLE_MASK
#define CH0_DMA_ENABLE_MASK		(1 << 0)
#endif
#ifndef CH0_DMA_START_MASK
#define CH0_DMA_START_MASK		(1 << 1)
#endif
#ifndef CH0_DMA_ABORT_MASK
#define CH0_DMA_ABORT_MASK		(1 << 2)
#endif
#ifndef CH0_DMA_CLEAR_IRQ_MASK
#define CH0_DMA_CLEAR_IRQ_MASK	(1 << 3)
#endif
#ifndef CH0_DMA_DONE_MASK
#define CH0_DMA_DONE_MASK		(1 << 4)
#endif

#ifndef DMA_PRESERVE_CHANNEL_MASK_1
#define DMA_PRESERVE_CHANNEL_MASK_1	(0xff << 8)
#endif
#ifndef CH1_DMA_ENABLE_MASK
#define CH1_DMA_ENABLE_MASK		(1 << 8)
#endif
#ifndef CH1_DMA_ENABLE_MASK
#define CH1_DMA_ENABLE_MASK		(1 << 9)
#endif
#ifndef CH1_DMA_ABORT_MASK
#define CH1_DMA_ABORT_MASK		(1 << 10)
#endif
#ifndef CH1_DMA_CLEAR_IRQ_MASK
#define CH1_DMA_CLEAR_IRQ_MASK	(1 << 11)
#endif
#ifndef CH1_DMA_DONE_MASK
#define CH1_DMA_DONE_MASK		(1 << 12)
#endif

#define DESCRIPTOR_IN_PCI_SPACE	(1 << 0)
#define END_OF_CHAIN_MARKER		(1 << 1) 
#define IRQ_AFTER_COUNT			(1 << 2)
#define TRANSFER_TO_PCI			(1 << 3)

// These are the masks for the PCI DMA Control Registers. 
#ifndef DMA_CMD_STAT_DONE
#define DMA_CMD_STAT_DONE      0x00000010
#endif
#ifndef DMA_CMD_STAT_CMD_MASK
#define DMA_CMD_STAT_CMD_MASK  0x00000007
#endif
#ifndef DMA_CMD_STAT_INT_CLEAR
#define DMA_CMD_STAT_INT_CLEAR 0x00000008
#endif
#ifndef DMA_CMD_STAT_START
#define DMA_CMD_STAT_START     0x00000003
#endif
#ifndef DMA_CMD_STAT_ABORT
#define DMA_CMD_STAT_ABORT     0x00000004
#endif
#ifndef DMA_MODE_DONE_ENABLE
#define DMA_MODE_DONE_ENABLE   0x00000400
#endif

// These are the masks for the PCI DMA Control Registers. 
#ifndef DMA_CMD_STAT_DONE_CH0
#define DMA_CMD_STAT_DONE_CH0      0x0010
#endif
#ifndef DMA_CMD_STAT_CMD_MASK_CH0
#define DMA_CMD_STAT_CMD_MASK_CH0  0x0007
#endif
#ifndef DMA_CMD_STAT_INT_CLEAR_CH0
#define DMA_CMD_STAT_INT_CLEAR_CH0 0x0008
#endif
#ifndef DMA_CMD_STAT_ABORT_CH0
#define DMA_CMD_STAT_ABORT_CH0     0x0004
#endif
#ifndef DMA_CMD_STAT_START_CH0
#define DMA_CMD_STAT_START_CH0     0x0003
#endif
#ifndef DMA_CMD_STAT_DONE_CH1
#define DMA_CMD_STAT_DONE_CH1      0x1000
#endif
#ifndef DMA_CMD_STAT_CMD_MASK_CH1
#define DMA_CMD_STAT_CMD_MASK_CH1  0x0700
#endif
#ifndef DMA_CMD_STAT_INT_CLEAR_CH1
#define DMA_CMD_STAT_INT_CLEAR_CH1 0x0800
#endif
#ifndef DMA_CMD_STAT_ABORT_CH1
#define DMA_CMD_STAT_ABORT_CH1     0x0400
#endif
#ifndef DMA_CMD_STAT_START_CH1
#define DMA_CMD_STAT_START_CH1     0x0300
#endif
#ifndef DMA_MODE_DONE_ENABLE
#define DMA_MODE_DONE_ENABLE       0x00000400
#endif


#endif // #ifndef PLX_REGS
