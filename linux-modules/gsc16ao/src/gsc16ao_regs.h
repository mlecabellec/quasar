// vim:ts=4 expandtab:
/*****************************************************************************
 *                                                                           *
 * File:         gsc16ao_regs.h                                              *
 *                                                                           *
 * Description:  The interface to the GSC16AO Linux device driver.           *
 *                                                                           *
 * Date:         5/20/2009                                                   *
 * History:                                                                  *
 *                                                                           *
 *   2  5/20/09 D. Dubash                                                    *
 *              new ioctl IOCTL_GSC16AO_SELECT_DIFFERENTIAL_SYNC_IO          *
 *              new ioctl IOCTL_GSC16AO_DISABLE_EXT_BURST_TRIGGER            *
 *                                                                           *
 *   1  5/30/03 G. Barton                                                    *
 *              Created                                                      *
 *                                                                           *
 *  Copyrights (c):                                                          *
 *      Concurrent Computer Corporation, 2003                                *
 *****************************************************************************/
#ifndef _GSC16AO_REGS
#define _GSC16AO_REGS

/*
 * PCI Configuration Registers
 */
#define DEVICE_VENDOR_ID              	    0
#define STATUS_COMMAND                      1
#define CLASS_CODE_REVISION_ID              2
#define BIST_HDR_TYPE_LAT_CACHE_SIZE        3
#define PCI_MEM_BASE_ADDR                   4
#define PCI_IO_BASE_ADDR                    5
#define PCI_BASE_ADDR_0                     6
#define PCI_BASE_ADDR_1                     7
#define CARDBUS_CIS_PTR                     10
#define SUBSYS_ID_VENDOR_ID                 11
#define PCI_BASE_ADDR_LOC_ROM               12
#define LAT_GNT_INT_PIN_LINE                15

/*
 * GSC16AO control registers (base = PCI BAR2)
 */
#define BOARD_CTRL_REG                	0
#define CHANNEL_SELECTION_REG         	1
#define RATE_CTRL_REG                	2
#define BUFFER_OPS_REG                 	3
#define RESERVED_1                      4
#define RESERVED_2                      5
#define OUTPUT_BUF_REG               	6
#define ADJUSTABLE_CLOCK   		7
#define LAST_LOCAL_REGISTER		7

/* Board control register */
#define BCR_BURST_ENABLED            	(1<<0)
#define BCR_BURST_READY              	(1<<1)
#define BCR_BURST_TRIGGER            	(1<<2)
#define BCR_REMOTE_GROUND_SENSE      	(1<<3)
#define BCR_OFFSET_BINARY            	(1<<4)
#define BCR_DIFFERENTIAL_SYNC_IO        (1<<5)
#define BCR_DISABLE_EXT_BURST_TRIGGER   (1<<6)
#define BCR_SIMULTANEOUS_OUTPUTS     	(1<<7)
#define BCR_IRQ_SHIFT                	8
#define BCR_IRQ_INIT                    (0<<BCR_IRQ_SHIFT)
#define BCR_IRQ_AUTOCAL_COMPLETE        (1<<BCR_IRQ_SHIFT)
#define BCR_IRQ_OUT_BUFFER_EMPTY        (2<<BCR_IRQ_SHIFT)
#define BCR_IRQ_OUT_BUFFER_LOW_QUARTER  (3<<BCR_IRQ_SHIFT)
#define BCR_IRQ_OUT_BUFFER_HIGH_QUARTER (4<<BCR_IRQ_SHIFT)
#define BCR_IRQ_BURST_TRIGGER_READY     (5<<BCR_IRQ_SHIFT)
#define BCR_IRQ_LOAD_READY              (6<<BCR_IRQ_SHIFT)
#define BCR_IRQ_END_LOAD_READY          (7<<BCR_IRQ_SHIFT)
#define BCR_IRQ_MASK 			        (7<<BCR_IRQ_SHIFT) 
                                            /* mask off all IRQ source bits */
#define BCR_INTERRUPT_REQUEST_FLAG   	(1<<11)

/* BCR calibration mode bits */
#define BCR_CALIBRATION_MODE_MASK 	    (3<<12)
#define BCR_NO_CALIBRATION         	    0
#define BCR_INIT_CALIBRATION          	(2<<12)
#define BCR_CALIBRATION_MODE_CM0  	    (1<<12)
#define BCR_CALIBRATION_MODE_CM1  	    (1<<13)

#define BCR_BIT_STATUS_FLAG          	(1<<14)
#define BCR_INITIALIZE               	(1<<15)

/* BCR output voltage range bits */
#define BCR_OUTPUT_RANGE                (3<<16)
#define BCR_OUTPUT_RANGE_FLV            (1<<16)

#define BCR_OUTPUT_RANGE_1_25V          (0 << 16)
#define BCR_OUTPUT_RANGE_2_5V           (1 << 16)
#define BCR_OUTPUT_RANGE_5V             (2 << 16)
#define BCR_OUTPUT_RANGE_10V            (3 << 16)

#define BCR_OUTPUT_RANGE_1_5V_FLV       (0 << 16)
#define BCR_OUTPUT_RANGE_2_5V_FLV       (1 << 16)
#define BCR_OUTPUT_RANGE_5V_FLV         (0 << 16)
#define BCR_OUTPUT_RANGE_10V_FLV        (1 << 16)

/* BCR output filter bits (FLV) */
#define BCR_OUTPUT_FILTER               (3 << 18)
#define BCR_OUTPUT_FILTER_NONE          (0 << 18)
#define BCR_OUTPUT_FILTER_NONE1         (1 << 18)
#define BCR_OUTPUT_FILTER_A             (2 << 18)
#define BCR_OUTPUT_FILTER_B             (3 << 18)

/* Channel selection register */
#define CSR_SELECT_ALL_CHANNELS 	0x7F
#define CSR_DESELECT_ALL_CHANNELS 	0x00
#define CSR_CHANNEL_00_ENABLED 		(1<<0)
#define CSR_CHANNEL_01_ENABLED 		(1<<1)
#define CSR_CHANNEL_02_ENABLED 		(1<<2)
#define CSR_CHANNEL_03_ENABLED 		(1<<3)
#define CSR_CHANNEL_04_ENABLED 		(1<<4)
#define CSR_CHANNEL_05_ENABLED 		(1<<5)
#define CSR_CHANNEL_06_ENABLED 		(1<<6)
#define CSR_CHANNEL_07_ENABLED 		(1<<7)
#define CSR_CHANNEL_08_ENABLED 		(1<<8)
#define CSR_CHANNEL_09_ENABLED 		(1<<9)
#define CSR_CHANNEL_10_ENABLED 		(1<<10)
#define CSR_CHANNEL_11_ENABLED 		(1<<11)

/* Output data buffer register */
#define ODB_DATA_MASK 			0xffff
#define ODB_EOF_FLAG 			(1<<16)

/* Sample rate register */
#define SRC_MAX				0xFFFF

/* Buffer operations register */
#define BOR_BUFFER_SIZE_MASK 		    0x0F

/* GSC16AO12 */
#define OUT_BUFFER_SIZE_AO12_4     		0x0
#define OUT_BUFFER_SIZE_AO12_8     		0x1
#define OUT_BUFFER_SIZE_AO12_16    		0x2 
#define OUT_BUFFER_SIZE_AO12_32    		0x3
#define OUT_BUFFER_SIZE_AO12_64    		0x4
#define OUT_BUFFER_SIZE_AO12_128   		0x5
#define OUT_BUFFER_SIZE_AO12_256   		0x6
#define OUT_BUFFER_SIZE_AO12_512   		0x7
#define OUT_BUFFER_SIZE_AO12_1024  		0x8
#define OUT_BUFFER_SIZE_AO12_2048  		0x9
#define OUT_BUFFER_SIZE_AO12_4096  		0xa
#define OUT_BUFFER_SIZE_AO12_8192  		0xb
#define OUT_BUFFER_SIZE_AO12_16384 		0xc
#define OUT_BUFFER_SIZE_AO12_32768 		0xd
#define OUT_BUFFER_SIZE_AO12_65536 		0xe
#define OUT_BUFFER_SIZE_AO12_131072		0xf

/* GSC16AO16 */
#define OUT_BUFFER_SIZE_AO16_8     		0x0
#define OUT_BUFFER_SIZE_AO16_16    		0x1
#define OUT_BUFFER_SIZE_AO16_32    		0x2 
#define OUT_BUFFER_SIZE_AO16_64    		0x3
#define OUT_BUFFER_SIZE_AO16_128   		0x4
#define OUT_BUFFER_SIZE_AO16_256   		0x5
#define OUT_BUFFER_SIZE_AO16_512   		0x6
#define OUT_BUFFER_SIZE_AO16_1024  		0x7
#define OUT_BUFFER_SIZE_AO16_2048  		0x8
#define OUT_BUFFER_SIZE_AO16_4096  		0x9
#define OUT_BUFFER_SIZE_AO16_8192  		0xa
#define OUT_BUFFER_SIZE_AO16_16384 		0xb
#define OUT_BUFFER_SIZE_AO16_32768 		0xc
#define OUT_BUFFER_SIZE_AO16_65536 		0xd
#define OUT_BUFFER_SIZE_AO16_131072		0xe
#define OUT_BUFFER_SIZE_AO16_262144		0xf

#define BOR_EXTERNAL_CLOCK      	(1<<4)
#define BOR_ENABLE_CLOCK        	(1<<5)
#define BOR_CLOCK_READY         	(1<<6)
#define BOR_SW_CLOCK            	(1<<7)
#define BOR_CIRCULAR_BUFFER     	(1<<8)
#define BOR_LOAD_REQUEST        	(1<<9)
#define BOR_LOAD_READY          	(1<<10)
#define BOR_CLEAR_BUFFER        	(1<<11)
#define BOR_BUFFER_EMPTY        	(1<<12)
#define BOR_BUFFER_LOW_QUARTER  	(1<<13)
#define BOR_BUFFER_HIGH_QUARTER 	(1<<14)
#define BOR_BUFFER_FULL         	(1<<15)

/* Adjustable clock register */
#define ACR_CLOCK_RATE_MASK 		0x1F
#define ACR_CLOCK_INITIATOR 		(1<<9)

/*
 * PLX registers (base = PCI BAR0)
 */

/* PLX DMA register offsets */
#define DMA_CH_0_MODE			(32)
#define DMA_CH_0_PCI_ADDR		(33)
#define DMA_CH_0_LOCAL_ADDR		(34)
#define DMA_CH_0_TRANS_BYTE_CNT		(35)
#define DMA_CH_0_DESC_PTR		(36)
#define DMA_CH_1_MODE			(37)
#define DMA_CH_1_PCI_ADDR		(38)
#define DMA_CH_1_LOCAL_ADDR		(39)
#define DMA_CH_1_TRANS_BYTE_CNT		(40)
#define DMA_CH_1_DESC_PTR		(41)
#define DMA_CMD_STATUS			(42)
#define DMA_MODE_ARBITRATION		(43)
#define DMA_THRESHOLD_REG		(44)

/* DMA command/status register bits for channel 0 */
#define DMA0_ENABLE			0x01
#define DMA0_START			0x02
#define DMA0_ABORT			0x04
#define DMA0_CLR_INT			0x08
#define DMA0_DONE			0x10

/* DMA command/status register bits for channel 1  */
#define DMA1_ENABLE			(0x01<<8)
#define DMA1_START			(0x02<<8)
#define DMA1_ABORT			(0x04<<8)
#define DMA1_CLR_INT			(0x08<<8)
#define DMA1_DONE			(0x10<<8)

/* DMA Command/Status Register Bit combinations */
#define START_DMA_CMD      		0x0000000B
#define STOP_DMA_CMD_MASK  		0x0000FF01
#define ABORT_DMA_CMD_MASK 		0x00000004

/* DMA Mode Register Masks */
#define DMA_MODE_DEFAULT       		0x00000D40
#define DMA_MODE_CHAINING      		0x00000200
#define DMA_MODE_DEMAND_ENABLE 		0x00001000

/* DMA Descriptor Pointer Register Masks */
#define DPR_RW_SHIFT            	3
#define DPR_INT_ON_DMA_COMPLETE 	0x0004
#define DPR_END_OF_CHAIN        	0x0002

/* PLX local configuration register offsets */
#define PCI_TO_LOC_ADDR_0_RNG		(0)
#define LOC_BASE_ADDR_REMAP_0		(1)
#define MODE_ARBITRATION		(2)
#define BIG_LITTLE_ENDIAN_DESC		(3)
#define PCI_TO_LOC_ROM_RNG		(4)
#define LOC_BASE_ADDR_REMAP_EXP_ROM	(5)
#define BUS_REG_DESC_0_FOR_PCI_LOC	(6)
#define DIR_MASTER_TO_PCI_RNG		(7)
#define LOC_ADDR_FOR_DIR_MASTER_MEM	(8)
#define LOC_ADDR_FOR_DIR_MASTER_IO	(9)
#define PCI_ADDR_REMAP_DIR_MASTER	(10)
#define PCI_CFG_ADDR_DIR_MASTER_IO	(11)

#define PCI_TO_LOC_ADDR_1_RNG		(60)
#define LOC_BASE_ADDR_REMAP_1		(61)
#define BUS_REG_DESC_1_FOR_PCI_LOC	(62)

/* PLX run time register offsets */
#define MAILBOX_REGISTER_0		(16)
#define MAILBOX_REGISTER_1		(17)
#define MAILBOX_REGISTER_2		(18)
#define MAILBOX_REGISTER_3		(19)
#define MAILBOX_REGISTER_4		(20)
#define MAILBOX_REGISTER_5		(21)
#define MAILBOX_REGISTER_6		(22)
#define MAILBOX_REGISTER_7		(23)
#define PCI_TO_LOC_DOORBELL		(24)
#define LOC_TO_PCI_DOORBELL		(25)
#define INT_CTRL_STATUS			(26)
#define PROM_CTRL_CMD_CODES_CTRL	(27)
#define DEVICE_ID_VENDOR_ID		(28)
#define REVISION_ID			(29)
#define MAILBOX_REG_0			(30)
#define MAILBOX_REG_1			(31)

/* PLX Interrupt Control/Status register masks */
#define IRQ_LOCAL_LSERR_ABORT		(1 << 0)
#define IRQ_LOCAL_LSERR_OVERFLOW	(1 << 1)
#define IRQ_GENERATE_SERR		(1 << 2)
#define IRQ_MAILBOX_ENABLE		(1 << 3)
#define IRQ_RESERVED1			(1 << 4)
#define IRQ_RESERVED2			(1 << 5)
#define IRQ_RESERVED3			(1 << 6)
#define IRQ_RESERVED4			(1 << 7)
#define IRQ_PCI_ENABLE			(1 << 8)
#define IRQ_PCI_DOORBELL_ENABLE		(1 << 9)
#define IRQ_ABORT_ENABLE		(1 << 10)
#define IRQ_LOCAL_PCI_ENABLE		(1 << 11)
#define IRQ_RETRY_ENABLE		(1 << 12)
#define IRQ_PCI_DOORBELL_ACTIVE		(1 << 13)
#define IRQ_ABORT_ACTIVE		(1 << 14)
#define IRQ_LOCAL_ACTIVE		(1 << 15)
#define IRQ_LOCAL_ENABLE		(1 << 16)
#define IRQ_LOCAL_DOORBELL_ENABLE	(1 << 17)
#define IRQ_DMA_0_ENABLE		(1 << 18)
#define IRQ_DMA_1_ENABLE		(1 << 19)
#define IRQ_LOCAL_DOORBELL_ACTIVE	(1 << 20)
#define IRQ_DMA_0_ACTIVE		(1 << 21)
#define IRQ_DMA_1_ACTIVE		(1 << 22)
#define IRQ_BIST_ACTIVE			(1 << 23)
#define IRQ_MASTER_ABORT		(1 << 24)
#define IRQ_DMA_0_ABORT			(1 << 25)
#define IRQ_DMA_1_ABORT			(1 << 26)
#define IRQ_TARGET_ABORT		(1 << 27)
#define IRQ_MAILBOX_0			(1 << 28)
#define IRQ_MAILBOX_1			(1 << 29)
#define IRQ_MAILBOX_2			(1 << 30)
#define IRQ_MAILBOX_3			(1 << 31)

/* IRQ reg bit combinations */
#define NON_DEMAND_DMA_MODE 		0x00020D43
#define DEMAND_DMA_MODE 		0x00001840
#define ENABLE_CHAINING 		(1 << 9)
#define DMA_PCI_IRQ_SELECT 		(1 << 17)

/* PLX Messaging Queue Register offsets */
#define OUT_POST_Q_INT_STATUS		(12)
#define OUT_POST_Q_INT_MASK          	(13)

#define IN_Q_PORT                    	(16)
#define OUT_Q_PORT                   	(17)

#define MSG_UNIT_CONFIG              	(48)
#define Q_BASE_ADDR                  	(49)
#define IN_FREE_HEAD_PTR             	(50)
#define IN_FREE_TAIL_PTR             	(51)
#define IN_POST_HEAD_PTR             	(52)
#define IN_POST_TAIL_PTR             	(53)
#define OUT_FREE_HEAD_PTR            	(54)
#define OUT_FREE_TAIL_PTR            	(55)
#define OUT_POST_HEAD_PTR            	(56)
#define OUT_POST_TAIL_PTR            	(57)
#define Q_STATUS_CTRL_REG            	(58)

#endif /* _GSC16AO_REGS */
