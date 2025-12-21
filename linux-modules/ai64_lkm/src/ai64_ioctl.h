// vim:ts=4 expandtab: 
/******************************************************************************
 *                                                                            *
 * File:        ai64_ioctl.h                                                  *
 *                                                                            *
 * Description: The interface to the AI64 Linux device driver.                *
 *                                                                            *
 * Date:        01/15/2009                                                    *
 * History:                                                                   *
 *                                                                            *
 *   7  01/15/09  D. Dubash                                                   *
 *      Changed define AI64_PLX_QSR                                           *
 *                                                                            *
 *   6  02/06/06  D. Dubash                                                   *
 *      Support for several new IOCTLS.                                       *
 *                                                                            *
 *   5  12/20/05  D. Dubash                                                   *
 *      Support for user allocation of DMA buffer.                            *
 *                                                                            *
 *   4  10/26/04  D. Dubash                                                   *
 *      Added new IOCTL_AI64_SET_POLAR, IOCTL_AI64_VALIDATE_CHAN_0,           *
 *      IOCTL_AI64_GET_BUFFER_HWM, IOCTL_AI64_ABORT_READ  and                 *
 *      IOCTL_AI64_CTRL_AUX_SYNC_IO ioctls and new #defines.                  *
 *                                                                            *
 *   3  07/29/04  D. Dubash                                                   *
 *      Added the new mmap structures. Several new ioctls.                    *
 *                                                                            *
 *   2  07/15/04   R. Calabro                                                 *
 *      Updated to run on RedHawk 1.4                                         *
 *                                                                            *
 *   1  Nov 2002   Evan Hillman                                               *
 *      This is the header used by AI64 applications.                         *
 *                                                                            *
 ******************************************************************************/
/***
 *** PCI16AI64_IOCTL.H 
 ***
 ***  General description of this file:
 ***  	IOCTL code and structure definitions for General Standards PCI-16AI64 
 ***  	16 channel PCI A/D board. This file is part of the Linux
 ***  	driver distribution for this board.
 ***  	
 ***  	This file is necessary to compile application programs, therefore 
 ***  	should be included in binary only driver distributions.
 ***
 ***	In the following, 'local' registers are board-specific, as
 ***	contrasted to the registers located on the PLX PCI bus
 ***	access chip.
 ***
 ***  Copyrights (c):
 ***  	General Standards Corporation (GSC), Sept 2003
 ***
 ***  Author:
 ***  	Evan Hillman, GSC Inc. (evan@generalstandards.com)
 ***
 ***  Support:
 ***  	Primary support for this driver is provided by GSC. 
 ***
 ***  Platform (tested on, may work with others):
 ***  	Linux, kernel version 2.2.x, Intel hardware.
 ***
 ***  Revision history:
 ***    Nov 2002: initial release
 ***/

#ifndef __AI64_IOCTL_H_INCLUDED__
#define __AI64_IOCTL_H_INCLUDED__

#define	AI64_MODULE_NAME		"ai64"

#define AI64_DEFAULT_TIMEOUT 10
#define AI64_DEFAULT_TIMEOUT_MSEC MSECS_TO_SLEEP(DEFAULT_TIMEOUT*1000)

/*
 * IOCTL_AI64_READ_REGISTER
 *
 * Parameter = ai64_registers *registers;
 */

struct ai64_registers
{
    unsigned int ai64_register;	/* RANGE: 0-94 */
    unsigned int register_value;	/* RANGE: 0x0-0xFFFFFFFF */
};

struct ai64_hwm
{
	unsigned int	total_buffers;
	unsigned int	buffers_inuse_hwm;
};

typedef struct {
    unsigned int        *phys_mem;          /* physical memory: physical address    */
    unsigned int        phys_mem_size;      /* physical memory: memory size - bytes */
} ai64_phys_mem_t;
 
#define	AI64_MAX_DMA_SAMPLES	(256*1024)	/* 256 K samples */
#define	AI64_MAX_CHANNELS		(64)		/* 64 channels */

/* IOCTL_MMAP_SELECT
 *
 * MMAP Selection Commands 
 */
typedef struct
{
    unsigned int select;        /* mmap() selection */
    unsigned long offset;       /* returned offset */
    unsigned long size;         /* returned size */
} ai64_mmap_select_t;

#define AI64_SELECT_GSC_MMAP      1         /* Select GSC Register */
#define AI64_SELECT_PLX_MMAP      2         /* Select PLX Register */
#define AI64_SELECT_PHYS_MEM_MMAP 3         /* MMAP physical memory */

#define AI64_GSC_MMAP_SIZE        256       /* GSC MMAP Size */
#define AI64_PLX_MMAP_SIZE        256       /* PLX MMAP Size */

/******************** Some Structures (drd) ***********************/
/*** Registers: GSC Control and Data Registers ***/
typedef volatile struct {
	uint	board_control;			/* Board Control Register (BCR) */
	uint	interrupt_control;		/* Interrupt conditions and flags */
	uint	input_data_buffer;		/* Analog Input Data Buffer */
	uint	input_buffer_control;	/* Input Buffer Threshold & Control*/
	uint	rate_a_generator;		/* Rate-A generator freq. selection */
	uint	rate_b_generator;		/* Rate-B generator freq. selection */
	uint	buffer_size;			/* Analog Input buffer size */
	uint	reserved_1;				/* 0x1c reserved */
	uint	scan_and_sync_control;	/* Ch/scan, clocking and sync src*/
	uint	reserved_2;				/* 0x24 reserved */
	uint	board_configuration;	/* Firmware revision and option straps */
	uint	autocal_values;			/* Autocal value readback */
	uint	auxiliary_rw_reg;		/* Auxiliary register. For internal use only */
	uint	auxiliary_sync_ctrl;	/* Controls auxiliary sync I/O port */
	uint	reserved[2];
} ai64_gsc_ctrl_data;

/*** Registers: PLX PCI9080: Local Configuration Registers ***/
typedef volatile struct {
	uint	lc_range_0;		/* Range for PCI-Local Address Space 0 */
	uint	lc_base_addr_0;	/* Local Base Address for PCI-Local Addr Space 0 */
	uint	lc_mode_arb;	/* Mode/arbitration */
	uint	lc_big_little;	/* Big/Little Endian Descriptor */
	uint	lc_erom_range_0;/* Range for PCI-Local Expansion ROM */
	uint	lc_erom_addr_0;	/* Local Base Addr for P2L Exp ROM and BREQ ctrl*/
	uint	lc_busr_desc_0;	/* Local Bus Region Descriptor Sp0 for P2L access */
	uint	lc_master_range;/* Range for direct master to PCI */
	uint	lc_mast_mem_lba;/* Local base address for Dir Master to PCI mem */
	uint	lc_mast_ioc_lba;/* Local base address for Dir Master to PCI IO/CFG*/
	uint	lc_mast_pci_ba;	/* PCI Base Address for Direct Master to PCI */
	uint	lc_mast_pci_cfg;/* PCI cfg. addr. for dir. master to PCI IO/CFG */
	uint	lc_reserved[48];/* reserved 0x30 - 0xEF */
	uint	lc_range_1;		/* Range for PCI-Local Address Space 1 */
	uint	lc_base_addr_1;	/* Local Base Address for PCI-Local Addr Space 1 */
	uint	lc_busr_desc_1;	/* Local Bus Region Descriptor Sp1 for P2L access */
} ai64_plx_local_cfg;

/*** Registers: PLX PCI9080: Run Time Registers ***/
typedef volatile struct {
	uint	rt_reserved[16];/* reserved 0x00 - 0x3F */
	uint	rt_mbox_reg_0;	/* Mailbox Register 0 */
	uint	rt_mbox_reg_1;	/* Mailbox Register 1 */
	uint	rt_mbox_reg_2;	/* Mailbox Register 2 */
	uint	rt_mbox_reg_3;	/* Mailbox Register 3 */
	uint	rt_mbox_reg_4;	/* Mailbox Register 4 */
	uint	rt_mbox_reg_5;	/* Mailbox Register 5 */
	uint	rt_mbox_reg_6;	/* Mailbox Register 6 */
	uint	rt_mbox_reg_7;	/* Mailbox Register 7 */
	uint	rt_p2l_doorbell;/* PCI to Local Doorbell */
	uint	rt_l2p_doorbell;/* Local to PCI Doorbell */
	uint	rt_intctl_sta;	/* Interrupt Control/Status */
	uint	rt_eeprom_ctl;	/* Serial EEPROM ctrl, PCI Cmd, Usr I/O + Init clt*/
	uint	rt_dev_vend_id;	/* Device ID/ Vendor ID */
	uint	rt_rev_id;		/* unused/Revision ID */
	uint	rt_mbox_alwys_0;/* Mailbox Register 0 - Always */
	uint	rt_mbox_alwys_1;/* Mailbox Register 1 - Always */
} ai64_plx_run_time;

/*** Registers: PLX PCI9080: DMA Registers ***/
typedef volatile struct {
	uint	dma_reserved[32];	/* reserved  0x00 - 0x7F */
	uint	dma_ch0_mode;		/* DMA channel 0 mode */
	uint	dma_ch0_pci_addr;	/* DMA channel 0 PCI address */
	uint	dma_ch0_loc_addr;	/* DMA channel 0 Local address */
	uint	dma_ch0_xfr_cnt;	/* DMA channel 0 Transfer byte count */
	uint	dma_ch0_descrpt;	/* DMA channel 0 Descriptor Pointer */
	uint	dma_ch1_mode;		/* DMA channel 1 mode */
	uint	dma_ch1_pci_addr;	/* DMA channel 1 PCI address */
	uint	dma_ch1_loc_addr;	/* DMA channel 1 Local address */
	uint	dma_ch1_xfr_cnt;	/* DMA channel 1 Transfer byte count */
	uint	dma_ch1_descrpt;	/* DMA channel 1 Descriptor Pointer */
	uint	dma_ch1_ch0_cs;		/* DMA Channel 1 & DMA Chan 0 command/status */
	uint	dma_mode_arb;		/* DMA Mode/Arbitration register */
	uint	dma_threshold;		/* DMA Threshold register */
} ai64_plx_dma;

/*** Registers: PLX PCI9080: Messaging Queue Registers ***/
typedef volatile struct {
	uint	mq_reserved[12];	/* reserved 0x00 - 0x2F */
	uint	mq_out_intsta;		/* Outbound Post Queue Interrupt Status */
	uint	mq_out_intmsk;		/* Outbound Post Queue Interrupt Mask */
	uint	mq_reserved1[2];	/* reserved 0x38 - 3F */
	uint	mq_in_port;			/* Inbound Queue Port */
	uint	mq_out_port;		/* Outbound Queue Port */
	uint	mq_reserved2[30];	/* reserved 0x48 - 0xBF */
	uint	mq_msg_cfg;			/* Messaging Unit Configuration Register */
	uint	mq_base_addr;		/* Queue Base Address Register */
	uint	mq_in_free_head;	/* Inbound Free Head Pointer Register */
	uint	mq_in_free_tail;	/* Inbound Free Tail Pointer Register */
	uint	mq_in_post_head;	/* Inbound Post Head Pointer Register */
	uint	mq_in_post_tail;	/* Inbound Post Tail Pointer Register */
	uint	mq_out_free_head;	/* Outbound Free Head Pointer Register */
	uint	mq_out_free_tail;	/* Outbound Free Tail Pointer Register */
	uint	mq_out_post_head;	/* Outbound Post Head Pointer Register */
	uint	mq_out_post_tail;	/* Outbound Post Tail Pointer Register */
	uint	mq_satus_ctrl;		/* Queue Status/Control Register */
} ai64_plx_msg_queue;

#define AI64_REGISTER_INDEX(r)   ((r) & ~AI64_REGISTER_TYPE_MASK)
#define AI64_REGISTER_TYPE(r)    ((r) & AI64_REGISTER_TYPE_MASK)
#define	AI64_REGISTER_TYPE_MASK	0xC0000000L

/* Registers: GSC specific registers */
#define	AI64_GSC_REGISTER	0x00000000L
#define AI64_GSC_BCR	    (AI64_GSC_REGISTER + 0)
#define AI64_GSC_ICR	    (AI64_GSC_REGISTER + 1)
#define AI64_GSC_IDBR	    (AI64_GSC_REGISTER + 2)
#define AI64_GSC_IBCR	    (AI64_GSC_REGISTER + 3)
#define AI64_GSC_RAGR	    (AI64_GSC_REGISTER + 4)
#define AI64_GSC_RBGR	    (AI64_GSC_REGISTER + 5)
#define AI64_GSC_BUFSZ	    (AI64_GSC_REGISTER + 6)
#define AI64_GSC_SSCR	    (AI64_GSC_REGISTER + 8)
#define AI64_GSC_BCFG	    (AI64_GSC_REGISTER + 10)
#define AI64_GSC_ACAL	    (AI64_GSC_REGISTER + 11)
#define AI64_GSC_AUXREG	    (AI64_GSC_REGISTER + 12)
#define AI64_GSC_AUXCTRL	(AI64_GSC_REGISTER + 13)

/* Register: PCI Configuration Registers */
#define	AI64_PCI_REGISTER		0x40000000L
#define AI64_PCI_IDR			(AI64_PCI_REGISTER + 0)
#define AI64_PCI_CR_SR			(AI64_PCI_REGISTER + 1)
#define AI64_PCI_REV_CCR		(AI64_PCI_REGISTER + 2)
#define AI64_PCI_CLSR_LTR_HTR_BISTR	(AI64_PCI_REGISTER + 3)
#define AI64_PCI_BAR0			(AI64_PCI_REGISTER + 4)
#define AI64_PCI_BAR1			(AI64_PCI_REGISTER + 5)
#define AI64_PCI_BAR2			(AI64_PCI_REGISTER + 6)
#define AI64_PCI_BAR3			(AI64_PCI_REGISTER + 7)
#define AI64_PCI_BAR4			(AI64_PCI_REGISTER + 8)
#define AI64_PCI_BAR5			(AI64_PCI_REGISTER + 9)
#define AI64_PCI_CIS			(AI64_PCI_REGISTER + 10)
#define AI64_PCI_SVID_SID		(AI64_PCI_REGISTER + 11)
#define AI64_PCI_ERBAR			(AI64_PCI_REGISTER + 12)
#define AI64_PCI_ILR_IPR_MGR_MLR	(AI64_PCI_REGISTER + 15)

/* Registers: PLX PCI9080: Local Configuration Registers */
#define	AI64_PLX_REGISTER   0x80000000L
#define AI64_PLX_LASORR	    (AI64_PLX_REGISTER + 0)
#define AI64_PLX_LASORR	    (AI64_PLX_REGISTER + 0)
#define AI64_PLX_LAS0BA	    (AI64_PLX_REGISTER + 1)
#define AI64_PLX_MARBR		(AI64_PLX_REGISTER + 2)
#define AI64_PLX_BIGEND	    (AI64_PLX_REGISTER + 3)
#define AI64_PLX_EROMRR	    (AI64_PLX_REGISTER + 4)
#define AI64_PLX_EROMBA	    (AI64_PLX_REGISTER + 5)
#define AI64_PLX_LBRD0		(AI64_PLX_REGISTER + 6)
#define AI64_PLX_DMRR		(AI64_PLX_REGISTER + 7)
#define AI64_PLX_DMLBAM	    (AI64_PLX_REGISTER + 8)
#define AI64_PLX_DMLBAI	    (AI64_PLX_REGISTER + 9)
#define AI64_PLX_DMPBAM	    (AI64_PLX_REGISTER + 10)
#define AI64_PLX_DMCFGA	    (AI64_PLX_REGISTER + 11)
#define AI64_PLX_LAS1RR	    (AI64_PLX_REGISTER + 60) /* drd */
#define AI64_PLX_LAS1BA	    (AI64_PLX_REGISTER + 61) /* drd */
#define AI64_PLX_LBRD1		(AI64_PLX_REGISTER + 62) /* drd */

/* Registers: PLX PCI9080: Runtime Registers */
#define AI64_PLX_MBOX0		(AI64_PLX_REGISTER + 16)
#define AI64_PLX_MBOX1		(AI64_PLX_REGISTER + 17)
#define AI64_PLX_MBOX2		(AI64_PLX_REGISTER + 18)
#define AI64_PLX_MBOX3		(AI64_PLX_REGISTER + 19)
#define AI64_PLX_MBOX4		(AI64_PLX_REGISTER + 20)
#define AI64_PLX_MBOX5		(AI64_PLX_REGISTER + 21)
#define AI64_PLX_MBOX6		(AI64_PLX_REGISTER + 22)
#define AI64_PLX_MBOX7		(AI64_PLX_REGISTER + 23)
#define AI64_PLX_P2LDBELL	(AI64_PLX_REGISTER + 24)
#define AI64_PLX_L2PDBELL	(AI64_PLX_REGISTER + 25)
#define AI64_PLX_INTCSR	    (AI64_PLX_REGISTER + 26)
#define AI64_PLX_CNTRL		(AI64_PLX_REGISTER + 27)
#define AI64_PLX_PCIHIDR	(AI64_PLX_REGISTER + 28)
#define AI64_PLX_PCIHREV	(AI64_PLX_REGISTER + 29)
#define AI64_PLX_MBOX0_ALT	(AI64_PLX_REGISTER + 30)
#define AI64_PLX_MBOX1_ALT	(AI64_PLX_REGISTER + 31)

/* Registers: PLX PCI9080: DMA Registers */
#define AI64_PLX_DMAMODE0	(AI64_PLX_REGISTER + 32)
#define AI64_PLX_DMAPADR0	(AI64_PLX_REGISTER + 33)
#define AI64_PLX_DMALADR0	(AI64_PLX_REGISTER + 34)
#define AI64_PLX_DMASIZ0	(AI64_PLX_REGISTER + 35)
#define AI64_PLX_DMADPR0	(AI64_PLX_REGISTER + 36)
#define AI64_PLX_DMAMODE1	(AI64_PLX_REGISTER + 37)
#define AI64_PLX_DMAPADR1	(AI64_PLX_REGISTER + 38)
#define AI64_PLX_DMALADR1	(AI64_PLX_REGISTER + 39)
#define AI64_PLX_DMASIZ1	(AI64_PLX_REGISTER + 40)
#define AI64_PLX_DMADPR1	(AI64_PLX_REGISTER + 41)
#define AI64_PLX_DMACSR01	(AI64_PLX_REGISTER + 42)
#define AI64_PLX_DMAARB	    (AI64_PLX_REGISTER + 43)
#define AI64_PLX_DMATHR	    (AI64_PLX_REGISTER + 44)

/* Registers: PLX PCI9080: Messageing Queue Registers */
#define AI64_PLX_OPLFIS	    (AI64_PLX_REGISTER + 12)
#define AI64_PLX_OPLFIM	    (AI64_PLX_REGISTER + 13)
#define AI64_PLX_IQP		(AI64_PLX_REGISTER + 16)
#define AI64_PLX_OQP		(AI64_PLX_REGISTER + 17)
#define AI64_PLX_MQCR		(AI64_PLX_REGISTER + 48)
#define AI64_PLX_QBAR		(AI64_PLX_REGISTER + 49)
#define AI64_PLX_IFHPR		(AI64_PLX_REGISTER + 50)
#define AI64_PLX_IFTPR		(AI64_PLX_REGISTER + 51)
#define AI64_PLX_IPHPR		(AI64_PLX_REGISTER + 52)
#define AI64_PLX_IPTPR		(AI64_PLX_REGISTER + 53)
#define AI64_PLX_OFHPR		(AI64_PLX_REGISTER + 54)
#define AI64_PLX_OFTPR		(AI64_PLX_REGISTER + 55)
#define AI64_PLX_OPHPR		(AI64_PLX_REGISTER + 56)
#define AI64_PLX_OPTPR		(AI64_PLX_REGISTER + 57)
#define AI64_PLX_QSR		(AI64_PLX_REGISTER + 58)

#define IOCTL_AI64_MAGIC   'a'

 /**
  ** IOCTL defines.  See below for API details.
  **
  **/

#define IOCTL_AI64_NO_COMMAND                _IO  (IOCTL_AI64_MAGIC,1)
#define IOCTL_AI64_INIT_BOARD                _IO  (IOCTL_AI64_MAGIC,2)
#define IOCTL_AI64_ENABLE_PCI_INTERRUPTS     _IO  (IOCTL_AI64_MAGIC,3)
#define IOCTL_AI64_GET_PHYSICAL_MEMORY       _IOR (IOCTL_AI64_MAGIC,4,ai64_phys_mem_t *)
#define IOCTL_AI64_SS_MODE_CONFIG            _IOW (IOCTL_AI64_MAGIC,6, unsigned long)
#define IOCTL_AI64_ENABLE_DMA                _IOW (IOCTL_AI64_MAGIC,7, unsigned long)
// #define IOCTL_AI64_REG_FOR_INT_NOTIFY     _IOWR(IOCTL_AI64_MAGIC,8, struct device_int_notify_params)
#define IOCTL_AI64_GET_DEVICE_ERROR          _IOR (IOCTL_AI64_MAGIC,9, unsigned long)
#define IOCTL_AI64_INPUT_RANGE_CONFIG        _IOW (IOCTL_AI64_MAGIC,10,unsigned long)
#define IOCTL_AI64_INPUT_MODE_CONFIG         _IOW (IOCTL_AI64_MAGIC,11,unsigned long)
#define IOCTL_AI64_INPUT_FORMAT_CONFIG       _IOW (IOCTL_AI64_MAGIC,12,unsigned long)
#define IOCTL_AI64_START_AUTOCAL		     _IO  (IOCTL_AI64_MAGIC,13)
#define IOCTL_AI64_START_SINGLE_SCAN         _IO  (IOCTL_AI64_MAGIC,14)
#define IOCTL_AI64_SET_SCAN_SIZE             _IOW (IOCTL_AI64_MAGIC,15,unsigned long)
#define IOCTL_AI64_SET_SCAN_CLOCK            _IOW (IOCTL_AI64_MAGIC,16,unsigned long)
#define IOCTL_AI64_DISABLE_PCI_INTERRUPTS    _IO  (IOCTL_AI64_MAGIC,17)
#define IOCTL_AI64_SET_B_CLOCK_SOURCE        _IOW (IOCTL_AI64_MAGIC,18,unsigned long)
#define IOCTL_AI64_SINGLE_CHAN_SELECT        _IOW (IOCTL_AI64_MAGIC,19,unsigned long)
#define IOCTL_AI64_SET_SCAN_RATE_A           _IOR (IOCTL_AI64_MAGIC,20,unsigned long)
#define IOCTL_AI64_SET_SCAN_RATE_B           _IOW (IOCTL_AI64_MAGIC,21,unsigned long)
#define IOCTL_AI64_GEN_DISABLE_A             _IO  (IOCTL_AI64_MAGIC,22)
#define IOCTL_AI64_GEN_DISABLE_B             _IO  (IOCTL_AI64_MAGIC,23)
#define IOCTL_AI64_GEN_ENABLE_A              _IO  (IOCTL_AI64_MAGIC,24)
#define IOCTL_AI64_GEN_ENABLE_B              _IO  (IOCTL_AI64_MAGIC,25)
#define IOCTL_AI64_SET_BUFFER_THRESHOLD      _IOW (IOCTL_AI64_MAGIC,26,unsigned long)
#define IOCTL_AI64_CLEAR_BUFFER              _IO  (IOCTL_AI64_MAGIC,27)
#define IOCTL_AI64_GET_DEVICE_TYPE           _IOR (IOCTL_AI64_MAGIC,28,unsigned long)
#define IOCTL_AI64_ENABLE_BUFFER_SS          _IO  (IOCTL_AI64_MAGIC,29)
#define IOCTL_AI64_DISABLE_BUFFER_SS         _IO  (IOCTL_AI64_MAGIC,30)

#define IOCTL_AI64_DUMP_REGISTERS			 _IO  (IOCTL_AI64_MAGIC,33)
#define IOCTL_AI64_GET_FIRMWARE_REV		     _IOR (IOCTL_AI64_MAGIC,34,unsigned long)
#define IOCTL_AI64_GET_DRIVER_INFO           _IOR (IOCTL_AI64_MAGIC,35,ai64_driver_info_t*)
#define IOCTL_AI64_MMAP_GET_OFFSET           _IOR (IOCTL_AI64_MAGIC,36,unsigned long *)
#define IOCTL_AI64_READ_REGISTER             _IOR(IOCTL_AI64_MAGIC,37,struct ai64_registers *)
#define IOCTL_AI64_WRITE_REGISTER            _IOR(IOCTL_AI64_MAGIC,38,struct ai64_registers *)
#define IOCTL_AI64_ENABLE_TEST_PATTERN       _IOW (IOCTL_AI64_MAGIC,39, unsigned long)
#define IOCTL_AI64_RETURN_16BIT_SAMPLES      _IOW (IOCTL_AI64_MAGIC,40, unsigned long)
#define IOCTL_AI64_SET_POLAR                 _IOW (IOCTL_AI64_MAGIC,41,unsigned long)
#define IOCTL_AI64_VALIDATE_CHAN_0           _IOW (IOCTL_AI64_MAGIC,42, unsigned long)
#define IOCTL_AI64_GET_BUFFER_HWM		     _IOR (IOCTL_AI64_MAGIC,43,unsigned long)
#define IOCTL_AI64_ABORT_READ                _IO  (IOCTL_AI64_MAGIC,44)
#define IOCTL_AI64_CTRL_AUX_SYNC_IO          _IOR (IOCTL_AI64_MAGIC,45,ai64_set_aux_sync_io_t*)
#define IOCTL_AI64_ALLOCATE_DMA_BUFFERS      _IOW (IOCTL_AI64_MAGIC,46, unsigned long)
#define IOCTL_AI64_GET_NUM_DMA_BUFFERS       _IOR (IOCTL_AI64_MAGIC,47, ai64_num_dma_bufs_t *)
#define IOCTL_AI64_REMOVE_DMA_BUFFERS        _IO  (IOCTL_AI64_MAGIC,48)
#define IOCTL_AI64_ADD_IRQ                   _IO  (IOCTL_AI64_MAGIC,49)
#define IOCTL_AI64_REMOVE_IRQ                _IO  (IOCTL_AI64_MAGIC,50)
#define IOCTL_AI64_GET_MASTER_CLOCK_FREQUENCY _IOR (IOCTL_AI64_MAGIC,51, double)
#define IOCTL_AI64_MMAP_SELECT               _IOR (IOCTL_AI64_MAGIC,52, ai64_mmap_select_t *)

/*****************************************************************************
 ** IOCTL_AI64_NO_COMMAND
 **
 ** Parameter = NONE
 **/

/*****************************************************************************
 ** IOCTL_AI64_INIT_BOARD
 **
 ** Parameter = NONE
 **/

/*****************************************************************************
** IOCTL_AI64_GET_DEVICE_TYPE	  	  
**
** returns an enumeration of the type of board the driver is attached to.
**
** Parameter = unsigned long *pType;
**/

enum type_index{
	gsc12ai64,
	gsc16ai64,
	gsc16ai64ss,
	gsc16ai64ssa_c
};

/*****************************************************************************
 ** IOCTL_AI64_READ_REGISTER	  
 ** IOCTL_AI64_WRITE_REGISTER	  
 **
 ** Parameter = AI64_DEVICE_REGISTER_PARAMS *pRegParams;
 **
 ** This structure is used to store information about a register. The
 ** IOCTL_AI64_READ_REGISTER and IOCTL_AI64_WRITE_REGISTER ioctl commands use this
 ** structure to read a particular register and to write a value into a particular
 ** register. 'DeviceRegister' stores the index of the register, range being 0-94, and
 ** 'ulRegisterValue' stores the register value, range being 0x0-0xFFFFFFFF.
 **/
typedef struct device_register_params {
  unsigned int DeviceRegister;	      	    /* range: 0-94, see definitions below */
  unsigned long ulValue;	    /* range: 0x0-0xFFFFFFFF, same register values below */
} AI64_DEVICE_REGISTER_PARAMS, *AI64_PDEVICE_REGISTER_PARAMS;


/* ==== Register Offsets ==== */
/* ---- 'local' Registers ---- */

#define AI64_BOARD_CTRL_REG			0
#define AI64_INT_CTRL_REG			1
#define AI64_INPUT_DATA_BUFF_REG    2
#define AI64_INPUT_BUFF_CTRL_REG	3
#define AI64_RATE_A_GENERATOR_REG	4
#define AI64_RATE_B_GENERATOR_REG	5
#define AI64_BUFFER_SIZE_REG		6
#define AI64_RESERVED_1_REG			7
#define AI64_SCAN_SYNCH_CTRL_REG	8
#define AI64_RESERVED_2_REG			9
#define AI64_BOARD_CONFIG_REG		10
#define AI64_AUTO_CAL_VALUES		11
#define AI64_AUX_RW					12
#define AI64_AUX_SYNC_IO_CTRL		13


/* ---- DMA Registers ---- */
/* ---- see PLX documentation for info ---- */

#define AI64_DMA_CH_0_MODE                	    32
#define AI64_DMA_CH_0_PCI_ADDR            	    33
#define AI64_DMA_CH_0_LOCAL_ADDR          	    34
#define AI64_DMA_CH_0_TRANS_BYTE_CNT      	    35
#define AI64_DMA_CH_0_DESC_PTR            	    36
#define AI64_DMA_CH_1_MODE                	    37
#define AI64_DMA_CH_1_PCI_ADDR            	    38
#define AI64_DMA_CH_1_LOCAL_ADDR          	    39
#define AI64_DMA_CH_1_TRANS_BYTE_CNT      	    40
#define AI64_DMA_CH_1_DESC_PTR            	    41
#define AI64_DMA_CMD_STATUS               	    42
#define AI64_DMA_MODE_ARB_REG             	    43
#define AI64_DMA_THRESHOLD_REG            	    44

/* ---- PCI Configuration Registers ---- */
/* ---- see PLX documentation for info ---- */

#define AI64_DEVICE_VENDOR_ID              	     0
#define AI64_STATUS_COMMAND                      1
#define AI64_CLASS_CODE_REVISION_ID              2
#define AI64_BIST_HDR_TYPE_LAT_CACHE_SIZE        3
#define AI64_PCI_MEM_BASE_ADDR                   4
#define AI64_PCI_IO_BASE_ADDR                    5
#define AI64_PCI_BASE_ADDR_0                     6
#define AI64_PCI_BASE_ADDR_1                     7
#define AI64_CARDBUS_CIS_PTR                     10
#define AI64_SUBSYS_ID_VENDOR_ID                 11
#define AI64_PCI_BASE_ADDR_LOC_ROM               12
#define AI64_LAT_GNT_INT_PIN_LINE                15

/* ---- Local Configuration Registers ---- */
/* ---- see PLX documentation for info ---- */

#define Ai64_PCI_TO_LOC_ADDR_0_RNG               0
#define Ai64_LOC_BASE_ADDR_REMAP_0               1
#define Ai64_MODE_ARBITRATION                    2
#define Ai64_BIG_LITTLE_ENDIAN_DESC              3
#define Ai64_PCI_TO_LOC_ROM_RNG                  4
#define Ai64_LOC_BASE_ADDR_REMAP_EXP_ROM         5
#define Ai64_BUS_REG_DESC_0_FOR_PCI_LOC          6
#define Ai64_DIR_MASTER_TO_PCI_RNG               7
#define Ai64_LOC_ADDR_FOR_DIR_MASTER_MEM         8
#define Ai64_LOC_ADDR_FOR_DIR_MASTER_IO          9
#define Ai64_PCI_ADDR_REMAP_DIR_MASTER           10
#define Ai64_PCI_CFG_ADDR_DIR_MASTER_IO          11
#define Ai64_PCI_TO_LOC_ADDR_1_RNG               92
#define Ai64_LOC_BASE_ADDR_REMAP_1               93
#define Ai64_BUS_REG_DESC_1_FOR_PCI_LOC          94

/* ---- Run Time Registers ---- */
/* ---- see PLX documentation for info ---- */

#define AI64_MAILBOX_REGISTER_0                  16
#define AI64_MAILBOX_REGISTER_1                  17
#define AI64_MAILBOX_REGISTER_2                  18
#define AI64_MAILBOX_REGISTER_3                  19
#define AI64_MAILBOX_REGISTER_4                  20
#define AI64_MAILBOX_REGISTER_5                  21
#define AI64_MAILBOX_REGISTER_6                  22
#define AI64_MAILBOX_REGISTER_7                  23
#define AI64_PCI_TO_LOC_DOORBELL                 24
#define AI64_LOC_TO_PCI_DOORBELL                 25
#define AI64_INT_CTRL_STATUS                     26
#define AI64_PROM_CTRL_CMD_CODES_CTRL            27
#define AI64_DEVICE_ID_VENDOR_ID                 28
#define AI64_REVISION_ID                         29
#define AI64_MAILBOX_REG_0                       30
#define AI64_MAILBOX_REG_1                       31

/* ---- Messaging Queue Registers ---- */
/* ---- see PLX documentation for info ---- */

#define AI64_OUT_POST_Q_INT_STATUS               12
#define AI64_OUT_POST_Q_INT_MASK                 13
#define AI64_IN_Q_PORT                           16
#define AI64_OUT_Q_PORT                          17
#define AI64_MSG_UNIT_CONFIG                     48
#define AI64_Q_BASE_ADDR                         49
#define AI64_IN_FREE_HEAD_PTR                    50
#define AI64_IN_FREE_TAIL_PTR                    51
#define AI64_IN_POST_HEAD_PTR                    52
#define AI64_IN_POST_TAIL_PTR                    53
#define AI64_OUT_FREE_HEAD_PTR                   54
#define AI64_OUT_FREE_TAIL_PTR                   55
#define AI64_OUT_POST_HEAD_PTR                   56
#define AI64_OUT_POST_TAIL_PTR                   57
#define AI64_Q_STATUS_CTRL_REG                   58

/* ==== Values for some key registers ==== */

/* ---- masks for the 'local' Board Control Register (32-bit addr 0). ---- */
/* ---- see GSC documentation for info ---- */

// analog input mode

#define AI64_BCR_INPUT_MODE                     0
#define AI64_BCR_SINGLE_ENDED                   1
#define AI64_BCR_ZERO_TEST                      2
#define AI64_BCR_VREF_TEST                      3

#define AI64_BCR_AIM_MASK                       0x0007
#define AI64_BCR_AIM_SHIFT                      0

// analog input - unipolar , bipolar 
#define AI64_BCR_POLAR_SHIFT				    3
#define	AI64_BCR_BI_POLAR						(0)
#define	AI64_BCR_UNI_POLAR						(1)
#define	AI64_BCR_POLAR_MASK						(1 << AI64_BCR_POLAR_SHIFT)

// analog input range

#define AI64_BCR_RANGE_SHIFT                    4

#define AI64_BCR_RANGE_2_5						(0)
#define AI64_BCR_RANGE_5	  					(1)
#define AI64_BCR_RANGE_10 						(2)

#define AI64_BCR_RANGE_MASK                     (3<<AI64_BCR_RANGE_SHIFT)

// offset binary mode 

#define AI64_BCR_OFFSET_BINARY_SHIFT            6
#define	AI64_BCR_TWOS_COMPLEMENT				(0)
#define	AI64_BCR_OFFSET_BINARY					(1)
#define AI64_BCR_FORMAT_MASK                    (1<<AI64_BCR_OFFSET_BINARY_SHIFT)

// External synch

#define AI64_BCR_ENABLE_EXTERN_SYNC             (1<<7)

// start single scan

#define AI64_BCR_INPUT_SYNCH_START				(1<<12)

// init autocalibration

#define AI64_BCR_AUTOCAL                        (1<<13)

// autocalibration test passed flag

#define AI64_BCR_AUTOCAL_PASS                   (1<<14)

// initialize the hardware

#define AI64_BCR_INITIALIZE                     (1<<15)


/* ---- masks for the 'local' Interrupt Control Register (32-bit addr 1). ---- */
/* ---- see GSC documentation for info ---- */

#define AI64_ICR_0_IDLE                         0
#define AI64_ICR_0_AUTOCAL_COMPLETE             1
#define AI64_ICR_0_INPUT_SCAN_INIT              2
#define AI64_ICR_0_INPUT_SCAN_COMPLETE          3
#define AI64_ICR_0_MASK                         0x07

#define AI64_ICR_0_REQUEST                      (1<<3)

#define AI64_ICR_1_IDLE                         (0)
#define AI64_ICR_1_LO_HI                        (1<<4)
#define AI64_ICR_1_HI_LO                        (2<<4)
#define AI64_ICR_1_MASK                         (0x03<<4)

#define AI64_ICR_1_REQUEST                      (1<<7)

/* ---- masks for the 'local' Input Data Register (32-bit addr 2). ---- */
/* ---- see GSC documentation for info ---- */

#define AI64_VALID_DATA_MASK                    0XFF

/* ---- masks for the 'local' Input Data Control Registers (32-bit addr 3). ---- */
/* ---- see GSC documentation for info ---- */

/* These settings are for the OLD 16AI64SSA cards */
#define AI64_IDC_INPUT_CLEAR_BUFFER             (1<<16)
#define AI64_IDC_THRESHOLD_FLAG                 (1<<17)
#define AI64_IDC_THRESHOLD_MASK                 0xFFFF

/* these for the SS boards only */
#define AI64_IDC_DISABLE_BUFFER_SS              (1<<18)
#define AI64_IDC_LARGE_FIFO_SS                  (1<<19)

/* These settings are for the NEW 16AI64SSA/C cards */
#define AI64_IDC_C_INPUT_CLEAR_BUFFER         (1<<18)
#define AI64_IDC_C_THRESHOLD_FLAG             (1<<19)
#define AI64_IDC_C_THRESHOLD_MASK             0x3FFFF

/* ---- masks for the 'local' Rate A Generator Register (32-bit addr 4). ---- */
/* ---- masks for the 'local' Rate B Generator Register (32-bit addr 5). ---- */
/* ---- see GSC documentation for info ---- */

#define AI64_RGR_NRATE_MASK                     (0xFFFF)
#define AI64_RGR_GEN_DISABLE                    (1<<16)

/* ---- masks for the 'local' Buffer Size (32-bit addr 6). ---- */
/* ---- see GSC documentation for info ---- */

/* no masks required... */

/* ---- masks for the 'local' Scan and Synch Control Register(32-bit addr 8). ---- */
/* ---- see GSC documentation for info ---- */

// Scan size defines

#define AI64_SSCR_1_CHAN_SIZE                   0
#define AI64_SSCR_2_CHAN_SIZE                   1
#define AI64_SSCR_4_CHAN_SIZE                   2
#define AI64_SSCR_8_CHAN_SIZE                   3
#define AI64_SSCR_16_CHAN_SIZE                  4
#define AI64_SSCR_32_CHAN_SIZE                  5
#define AI64_SSCR_64_CHAN_SIZE                  6

// #define AI64_SSCR_CHAN_SIZE_MASK 0x03
#define AI64_SSCR_CHAN_SIZE_MASK                0x07

// analog inputs scan clock

#define AI64_SSCR_CLOCK_SHIFT                   3
#define AI64_SSCR_SCAN_CLOCK_MASK               (3<<AI64_SSCR_CLOCK_SHIFT)

#define AI64_SSCR_INTERNAL_RATE_A               0
#define AI64_SSCR_INTERNAL_RATE_B               (1)
#define AI64_SSCR_EXTERNAL_SYNCH                (2)
#define AI64_SSCR_BCR_INPUT_SYNCH               (3)

// single channel defines

#define AI64_SSCR_SINGLE_CHANNEL_SHIFT          (12)
#define AI64_SSCR_SINGLE_CHANNEL_MASK (0x3F<<AI64_SSCR_SINGLE_CHANNEL_SHIFT)

// Rate "B" clock source

#define AI64_SSCR_RATE_B_CLOCK_SOURCE_SHIFT     (10)
#define AI64_SSCR_RATE_B_CLOCK_SOURCE_MASK (1<<AI64_SSCR_RATE_B_CLOCK_SOURCE_SHIFT)

#define AI64_SSCR_MASTER_CLOCK                  0
#define AI64_SSCR_RATE_A_GENERATOR              (1)

/* ---- DMA command/status register bits for channel 0 ---- */
/* ---- see PLX documentation for info ---- */

#define AI64_DMA0_ENABLE		                0x0001
#define AI64_DMA0_START			                0x0002
#define AI64_DMA0_ABORT			                0x0004
#define AI64_DMA0_CLR_INT		                0x0008
#define AI64_DMA0_DONE			                0x0010

/*** AUX SYNC I/O CONTROL ***/
#define	AI64_AUX_0_CTRL_SHIFT			        0
#define	AI64_AUX_0_CTRL_MASK			        0x00000003
#define	AI64_AUX_1_CTRL_SHIFT			        2
#define	AI64_AUX_1_CTRL_MASK			        0x0000000C
#define	AI64_AUX_2_CTRL_SHIFT			        4
#define	AI64_AUX_2_CTRL_MASK			        0x00000030
#define	AI64_AUX_3_CTRL_SHIFT			        6
#define	AI64_AUX_3_CTRL_MASK			        0x000000C0
#define	AI64_AUX_INVERT_INP_SHIFT		        8
#define	AI64_AUX_INVERT_INP_MASK		        0x00000100
#define	AI64_AUX_INVERT_OUT_SHIFT		        9
#define	AI64_AUX_INVERT_OUT_MASK		        0x00000200
#define	AI64_AUX_NOISE_SUP_SHIFT		        10
#define	AI64_AUX_NOISE_SUP_MASK			        0x00000400


/*****************************************************************************
 ** IOCTL_AI64_ENABLE_DMA
 **
 ** Parameter = unsigned long *DMAmode;
 **
 ** Set up to use DMA for data transfer.
 **/

enum {
	AI64_DMA_DISABLE,
	AI64_DMA_ENABLE,
	AI64_DMA_DEMAND_MODE,
	AI64_DMA_CONTINUOUS
} AI64_DRIVER_READ_MODE;

#define AI64_DMA_SAMPLE_SIZE                    (1024 * 32)

/*****************************************************************************
 ** IOCTL_AI64_GET_DEVICE_ERROR
 **
 ** Parameter = unsigned long *pulDeviceError;
 ** Return value range: 0-12, see codes below
 **/

#define AI64_DEVICE_SUCCESS                         0
#define AI64_DEVICE_INVALID_PARAMETER               1
#define AI64_DEVICE_INVALID_BUFFER_SIZE             2
#define AI64_DEVICE_PIO_TIMEOUT                     3
#define AI64_DEVICE_DMA_TIMEOUT                     4
#define AI64_DEVICE_IOCTL_TIMEOUT                   5
#define AI64_DEVICE_OPERATION_CANCELLED             6
#define AI64_DEVICE_RESOURCE_ALLOCATION_ERROR       7
#define AI64_DEVICE_INVALID_REQUEST                 8
#define AI64_DEVICE_AUTOCAL_FAILED                  9
#define AI64_DEVICE_DMA_CONT_TIMEOUT               10
#define AI64_DEVICE_SIGNAL                         11
#define AI64_DEVICE_DMA_BUFFER                     12
#define AI64_DEVICE_FIFO_OVERFLOW                  13
#define AI64_DEVICE_RESOURCE_IN_USE                14

/*****************************************************************************
 ** IOCTL_AI64_INPUT_RANGE_CONFIG
 **
 ** Parameter = unsigned long *pInputRange;
 ** Range: 0-2, see codes below
 **/

/* ---- scan mode codes ---- */
#define AI64_INPUT_RANGE_2_5                        0
#define AI64_INPUT_RANGE_5                          1
#define AI64_INPUT_RANGE_10                         3

/*****************************************************************************
 **  IOCTL_AI64_INPUT_MODE_CONFIG
 **
 **  Parameter = unsigned long *iMode
 **/

/* ---- mode config codes ---- */
#define AI64_INPUT_MODE_SHIFT	                    0
#define AI64_INPUT_MODE_MASK		                0x07

// #define AI64_INPUT_DIFFERENTIAL                  0
// #define AI64_INPUT_SINGLE_ENDED                  1
#define AI64_INPUT_SYSTEM_ANALOG_SS                 0
#define AI64_INPUT_RES1		                        1
#define AI64_INPUT_ZERO_TEST                        2
#define AI64_INPUT_VREF_TEST                        3

/*****************************************************************************
** IOCTL_AI64_SS_MODE_CONFIG
**
**  Parameter = unsigned long *iMode
**/
#define AI64_SS_SINGLE_ENDED			            0
#define AI64_SS_PSEUDO_DIFFERENTIAL	                1
#define AI64_SS_FULL_DIFFERENTIAL	                2

#define AI64_SS_MODE_SHIFT                          8
#define AI64_SS_MODE_MASK                           (3 << 8)


/*****************************************************************************
 **  IOCTL_AI64_SET_POLAR
 **
 **  Parameter = unsigned long *format
 ** Range: 0-1, see codes below
 **/

/* ---- device test codes ---- */
#define AI64_FORMAT_BI_POLAR                        0
#define AI64_FORMAT_UNI_POLAR                       1

/*****************************************************************************
 **  IOCTL_AI64_INPUT_FORMAT_CONFIG
 **
 **  Parameter = unsigned long *format
 ** Range: 0-1, see codes below
 **/

/* ---- device test codes ---- */
#define AI64_FORMAT_TWO_COMPLEMENT                  0
#define AI64_FORMAT_OFFSET_BINARY                   1

/*****************************************************************************
 **  IOCTL_AI64_START_AUTOCAL
 **
 **  Parameter =  NONE
 **/

/*****************************************************************************
 ** IOCTL_AI64_START_SINGLE_SCAN
 **
 ** Parameter = NONE;
 **/

/*****************************************************************************
 ** IOCTL_AI64_SET_SCAN_SIZE
 **
 ** Parameter = unsigned long *pScanSize;
 ** Range: 0-6, see codes below
 **/

/* ---- scan size codes ---- */
#define AI64_SCAN_SINGLE_CHAN	                    0
#define AI64_SCAN_2_CHAN                            1
#define AI64_SCAN_4_CHAN	                        2
#define AI64_SCAN_8_CHAN	                        3
#define AI64_SCAN_16_CHAN		                    4
#define AI64_SCAN_32_CHAN		                    5
#define AI64_SCAN_64_CHAN		                    6

/*****************************************************************************
 ** IOCTL_AI64_SET_SCAN_CLOCK
 **
 ** select the analog input scan clock
 **
 ** Parameter = unsigned long *clock
 ** range=0-3
 **/

#define AI64_INTERNAL_RATE_A	                    0
#define AI64_INTERNAL_RATE_B	                    1
#define AI64_EXTERNAL_SYNCH	                        2
#define AI64_BCR_INPUT_SYNCH	                    3

/*****************************************************************************
 **  IOCTL_AI64_DISABLE_PCI_INTERRUPTS
 **
 **  Parameter = NONE
 **/

/*****************************************************************************
 ** IOCTL_AI64_SET_B_CLOCK_SOURCE
 **
 ** Parameter = unsigned long *pSource;
 ** Range: 0-1, see codes below
 **/

#define AI64_MASTER_CLOCK                           0
#define AI64_RATE_A_OUTPUT                          1

/*****************************************************************************
 ** IOCTL_AI64_SINGLE_CHAN_SELECT
 **
 ** Parameter =  unsigned long *channel;
 ** Range: 0-63
 **
 **/

/*****************************************************************************
 ** IOCTL_AI64_SET_SCAN_RATE_A
 **
 ** Parameter = unsigned long *setting;
 ** Range = 0-0xFFFF
 **
 **/

/*****************************************************************************
 ** IOCTL_AI64_SET_SCAN_RATE_B
 **
 ** Parameter = unsigned long *setting;
 ** Range = 0-0xFFFF
 **
 **/

/*****************************************************************************
**  IOCTL_AI64_GEN_DISABLE_A
**
**  Parameter = NONE
**/

/*****************************************************************************
**  IOCTL_AI64_GEN_DISABLE_B
**
**  Parameter = NONE
**/

/*****************************************************************************
**  IOCTL_AI64_GEN_ENABLE_A
**
**  Parameter = NONE
**/

/*****************************************************************************
**  IOCTL_AI64_GEN_ENABLE_B
**
**  Parameter = NONE
**/

/*****************************************************************************
**  IOCTL_AI64_SET_BUFFER_THRESHOLD
**
**  Parameter = unsigned long * threshold
**  Range = 0..0xFFFF
**/

/*****************************************************************************
**  IOCTL_AI64_CLEAR_BUFFER
**
**  Parameter = NONE
**/

/*****************************************************************************
**  IOCTL_AI64_ENABLE_BUFFER_SS
**
**  Parameter = NONE
**/

/*****************************************************************************
**  IOCTL_AI64_DISABLE_BUFFER_SS
**
**  Parameter = NONE
**/

/*****************************************************************************
**  IOCTL_AI64_GET_DRIVER_INFO
**
**  Parameter = ai64_driver_info_t*
**/
typedef struct
{
	char    version[12];
	char    built[32];
	char	module_name[16];
} ai64_driver_info_t;

/*****************************************************************************
**  IOCTL_AI64_GET_NUM_DMA_BUFFERS
**
**  Parameter = ai64_num_dma_bufs_t*
**/
typedef struct
{
    uint    num_dma_buffers;
    uint    use_resmem;    /* 0= allocated, 1= reserved */
} ai64_num_dma_bufs_t;


/*****************************************************************************/
/* Auxiliary Sync I/O  - AI64SS */
#define AI64_AUX_DISABLE                        0
#define AI64_AUX_INPUT                          1
#define AI64_AUX_OUTPUT                         2

#define AI64_AUX_FALSE                          0
#define AI64_AUX_TRUE                           1

/*****************************************************************************
**  IOCTL_AI64_SET_AUX_SYNC_IO
**
**  Parameter = ai64_set_aux_sync_io_t
**/

typedef struct
{
	char	write;	/* 0 = reads register, 1 = sets register */
	char    aux_0;
	char    aux_1;
	char    aux_2;
	char    aux_3;
	char	invert_inputs;
	char	invert_outputs;
	char	noise_suppress;
} ai64_set_aux_sync_io_t;

/*****************************************************************************
**
**  SP50 Related
**
**/
/* This is the value for the Previous AI64SSA cards */
//#define AI64_MASTER_CLOCK_FREQ              30000000.0  /* 30.0000 MHz */
#define	AI64_MAX_CLOCK_FREQ		    200000.0	/* 200 KHz */

/* This is the value for the NEW AI64SSA/C cards */

#endif	/* entire file */

