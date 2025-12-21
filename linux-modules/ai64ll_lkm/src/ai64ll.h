/******************************************************************************
 *                                                                            *
 * File:        ai64ll.h                                                      *
 *                                                                            *
 * Description: The interface to the AI64LL Linux device driver.              *
 *                                                                            *
 * Date:       02/15/2018                                                     *
 * History:                                                                   *
 *                                                                            *
 *  23  02/15/18 P. Ongini                                                    *
 *      Added Support for RH7.2                                               *
 *                                                                            *
 *  22  05/18/17 P. Ongini                                                    *
 *      Added Support for RH7.0                                               *
 *                                                                            *
 *  21  08/07/15 P. Ongini                                                    *
 *      Updated driver revision                                               *
 *                                                                            *
 *  20  12/08/14 P. Ongini                                                    *
 *      Added Support for RH6.5                                               *
 *                                                                            *
 *  19  11/10/14 P. Ongini                                                    *
 *      Remove ".x" and added architecture to version.                        *
 *                                                                            *
 *  18  03/31/14 D. Dubash                                                    *
 *      Fix bug in ai64ll_analog_in.c test                                    *
 *                                                                            *
 *  17  11/04/13 D. Dubash                                                    *
 *      Support for RH 6.3.6                                                  *
 *                                                                            *
 *  16  01/16/13 D. Dubash                                                    *
 *      Support for RH 6.3.3                                                  *
 *                                                                            *
 *  15  11/26/12 D. Dubash                                                    *
 *      Support for RH 6.3.1                                                  *
 *                                                                            *
 *  14  11/12/12 D. Dubash                                                    *
 *      Support for RH 6.0.4                                                  *
 *                                                                            *
 *  13  5/22/12 D. Dubash                                                     *
 *      Support for RH 5.4.15                                                 *
 *                                                                            *
 *  12  7/07/11 D. Dubash                                                     *
 *      Support for RH 5.4.12                                                 *
 *                                                                            *
 *  11  9/15/10 D. Higgins                                                    *
 *      Added support for RH 5.4.3, 5.4.4, 5.4.5                              *
 *                                                                            *
 *  10  5/11/10 D. Dubash                                                     *
 *      Support for RH 5.4.x                                                  *
 *                                                                            *
 *  09  06/26/2009 D. Dubash                                                  * 
 *      Fixed AI64LL/AI64 detection method as it was failing detection of     *
 *      an AI64 card because the card had a non-zero conversion value in it.  *
 *                                                                            *
 *   8  2/24/09 D. Dubash                                                     *
 *      Removed Architecture - placed in SUPPORTED_ARCHS file                 *
 *                                                                            *
 *   7  1/15/09 D. Dubash                                                     *
 *      Support for RH 5.2.x                                                  *
 *                                                                            *
 *   6  9/02/08 D. Dubash                                                     *
 *      Support for RH 5.1.1                                                  *
 *                                                                            *
 *   5  3/07/07 D. Dubash                                                     *
 *      Support for 64 bit architecture.                                      *
 *                                                                            *
 *   4  3/01/07 D. Dubash                                                     *
 *      Use converter counter to distinguish between AI64 and AI64LL cards.   *
 *                                                                            *
 *   3  2/09/07 D. Dubash                                                     *
 *      Added support for rpm                                                 *
 *                                                                            *
 *   2  1/11/07 D. Dubash                                                     *
 *      Added AI64LL_FAULT and AI64LL_GSC_IBCR definition                     *
 *      Support for new ioctl IOCTL_AI64LL_MMAP_GET_OFFSET which replaces     *
 *      ioctl IOCTL_MMAP_SELECT                                               *
 *                                                                            *
 *   1  7/10/03 D. Dubash                                                     *
 *      This is the header used by AI64LL applications.                       *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *  Copyright (C) 2003 and beyond Concurrent Real-Time, Inc.                  *
 *  All rights reserved                                                       *
 *                                                                            *
 ******************************************************************************/

#ifndef __AI64LL__
#define __AI64LL__

/*****************************************************************
 *   DO NOT CHANGE THESE NAMES AS THEY ARE SEARCHED BY build_rpm *
 *****************************************************************/
#define AI64LL_RedHawk_VERSION       "7.2_0"
/*************************************************************/

/* module name(s) 
#ifdef CONFIG_SUSE_KERNEL
    #define AI64LL_VERSION  AI64LL_SLERT_VERSION
#else
 */
    #define AI64LL_VERSION  AI64LL_RedHawk_VERSION
/* #endif */

#define AI64LL_MMAP_SUPPORT        /* drd */

#include <linux/version.h>


#include <linux/ioctl.h>

/*******************************************************************
 *** The defines below are being obsoleted and replaced with new ***
 *** defines. We will keep them for now and remove that in later ***
 *** releases.                                                   ***
 *******************************************************************/
#define REGISTER_INDEX          AI64LL_REGISTER_INDEX
#define REGISTER_TYPE           AI64LL_REGISTER_TYPE

#define	AI64LL_REGISTER_INDEX(r)	((r) & ~AI64LL_REGISTER_TYPE_MASK)
#define	AI64LL_REGISTER_TYPE(r)	((r) & AI64LL_REGISTER_TYPE_MASK)

#define	AI64LL_MASTER_CLOCK		24000000	/* 24MHz */

#define AI64LL_DEVICE_ID   0x9080
#define AI64LL_VENDOR_ID   0x10B5

#define AI64LL_SUBSYS_ID   0x240710B5

#define AI64LL_MAX_CHANS		64	/* maximum number of channels */

/******************** Some Structures (drd) ***********************/
/*** Registers: GSC Control and Data Registers ***/
typedef volatile struct {
	uint	board_control;		    /* Board Control Register (BCR) */
	uint	interrupt_control;	    /* Interrupt conditions and flags */
	uint	reserved_08;	    	/* Analog Input Data Buffer */
	uint	reserved_0c;   			/* Input Buffer Threshold & Control*/
	uint	reserved_10;		    /* reserved */
	uint	reserved_14;		    /* reserved */
	uint	conv_counter;		    /* Conversion Counter */
	uint	reserved_1c;		    /* reserved */
	uint	scan_and_sync_control;	/* Ch/scan, clocking and sync src*/
	uint	reserved_24;	        /* reserved */
	uint	firmware_rev;	        /* Firmware Revision */
	uint	autocal_values;         /* AutoCal Values */
	uint	reserved_30_fc[52];     /* reserved */
	uint   input_data_buffer[AI64LL_MAX_CHANS];	/* channels 00 to 63 */
} ai64ll_gsc_ctrl_data;

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
} ai64ll_plx_local_cfg;

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
} ai64ll_plx_run_time;

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
} ai64ll_plx_dma;

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
} ai64ll_plx_msg_queue;


/* ****************** IOCTL Commands ********************* */

#define IOCTL_AI64LL	's'

#define IOCTL_AI64LL_NO_COMMAND                _IO(IOCTL_AI64LL,1)
#define IOCTL_AI64LL_READ_REGISTER             _IOR(IOCTL_AI64LL,2,struct ai64ll_registers *)
#define IOCTL_AI64LL_WRITE_REGISTER            _IOW(IOCTL_AI64LL,3,struct ai64ll_registers *)
#define IOCTL_AI64LL_REQ_INT_NOTIFY            _IOW(IOCTL_AI64LL,4,ai64ll_int_info_t)
#define IOCTL_AI64LL_REQ_INT_STATUS            _IOR(IOCTL_AI64LL,5,ai64ll_int_info_t)
#define IOCTL_AI64LL_SET_INPUT_MODE            _IOW(IOCTL_AI64LL,6,unsigned long)
#define IOCTL_AI64LL_SET_RANGE		      _IOW(IOCTL_AI64LL,7,unsigned long)
#define IOCTL_AI64LL_SET_DATA_FORMAT           _IOW(IOCTL_AI64LL,8,unsigned long)
#define IOCTL_AI64LL_AUTO_CAL                  _IO(IOCTL_AI64LL,12)
#define IOCTL_AI64LL_INITIALIZE                _IO(IOCTL_AI64LL,13)
#define IOCTL_AI64LL_SET_INPUT_SCAN	      _IOW(IOCTL_AI64LL,23,struct ai64ll_inputscan)
#define IOCTL_AI64LL_GET_DEVICE_ERROR          _IOW(IOCTL_AI64LL,30,unsigned long)
#define IOCTL_AI64LL_READ_DEBUG	 	     _IOR(IOCTL_AI64LL,32,unsigned long*)
#define IOCTL_AI64LL_GET_DRIVER_INFO    _IOR(IOCTL_AI64LL,33,ai64ll_driver_info_t*)

#ifdef AI64LL_MMAP_SUPPORT   /* drd - MMAP SUPPORT */
//#define IOCTL_MMAP_SELECT  _IOR(IOCTL_AI64LL,34,unsigned long *) /*OBSOLETE*/
#define IOCTL_AI64LL_MMAP_GET_OFFSET    _IOR(IOCTL_AI64LL,35,unsigned long *)
#endif /* end AI64LL_MMAP_SUPPORT */




/*
 * Structures used by the IOCTL commands
 */

/*
 * IOCTL_AI64LL_NO_COMMAND
 *
 * Parameter = NONE
 */

/*
 * IOCTL_AI64LL_READ_REGISTER          &
 * IOCTL_AI64LL_WRITE_REGISTER         &
 *
 * Parameter = ai64ll_registers *registers;
 */

struct ai64ll_registers
{
    unsigned int ai64ll_register;	/* RANGE: 0-94 */
    unsigned int register_value;	/* RANGE: 0x0-0xFFFFFFFF */
};

/*
 * IOCTL_AI64LL_REQ_INT_NOTIFY
 *
 * Parameter = ai64ll_int_info_t *int_notify;
 */

/* eIRQ0IntCondition */
#define IRQ0_NO_INT                              0x00000000
#define IRQ0_AUTOCAL_COMPLETE                    0x00000001

typedef struct
{
	unsigned long  init;	// 0 = disable, else enable
	unsigned long  irq0;	// RANGE: 0-3
} ai64ll_int_info_t;

/*
 * IOCTL_AI64LL_REQ_INT_STATUS
 *
 * Parameter = ai64ll_int_info_t*
 */

/*
 * IOCTL_AI64LL_SET_INPUT_MODE
 *
 * Parameter = unsigned long *inputmode;
 *     RANGE: 0-7
 */

#define AI64LL_SS_DIFFERENTIAL  0
#define AI64LL_SS_SINGLE_ENDED	1
#define AI64LL_ZERO_TEST	    2
#define AI64LL_VREF_TEST	    3
#define MODE_DIFFERENTIAL 		AI64LL_SS_DIFFERENTIAL
#define MODE_SINGLE_ENDED 		AI64LL_SS_SINGLE_ENDED
#define MODE_ZERO_TEST    		AI64LL_ZERO_TEST
#define MODE_VREF_TEST    		AI64LL_VREF_TEST

/*
 * IOCTL_AI64LL_SET_RANGE
 *
 * Parameter = unsigned long *inputrange;
 *     RANGE: 0-2
 */

#define AI64LL_INPUT_RANGE_2_5		0
#define AI64LL_INPUT_RANGE_5		1
#define AI64LL_INPUT_RANGE_10		2
#define RANGE_2p5V  				AI64LL_INPUT_RANGE_2_5
#define RANGE_5V    				AI64LL_INPUT_RANGE_5
#define RANGE_10V   				AI64LL_INPUT_RANGE_10

/*
 * IOCTL_AI64LL_SET_DATA_FORMAT
 *
 * Parameter = unsigned long *data_format;
 *     RANGE: 0-1
 */
#define AI64LL_FORMAT_TWO_COMPLEMENT	0
#define AI64LL_FORMAT_OFFSET_BINARY		1
#define FORMAT_TWOS_COMPLEMENT 			AI64LL_FORMAT_TWO_COMPLEMENT
#define FORMAT_OFFSET_BINARY   			AI64LL_FORMAT_OFFSET_BINARY

#define AI64LL_RESOLUTION     (double)(unsigned int)(1 << 16)

#define AI64LL_FORMAT_BI_POLAR                        0
#define AI64LL_FORMAT_UNI_POLAR                       1

/*
 * IOCTL_AI64LL_AUTO_CAL
 *
 * Parameter = NONE
 */

/*
 * IOCTL_AI64LL_INITIALIZE
 *
 * Parameter = NONE
 */

/*
 * IOCTL_AI64LL_SET_INPUT_BUFFER_THRESHOLD
 *
 * Parameter = unsigned long *inbuf_threshold;
 *     RANGE: 0x0 - 0x7FFF
 */

/*
 * IOCTL_AI64LL_CLEAR_INPUT_BUFFER
 *
 * Parameter = NONE
 */

/*
 * IOCTL_AI64LL_SET_INPUT_SCAN
 *
 * Parameter = ai64ll_inputscan *inputscan;
 */

/* scan size options */
#define SCAN_SIZE_1_CHAN  0
#define SCAN_SIZE_2_CHAN  1
#define SCAN_SIZE_4_CHAN  2
#define SCAN_SIZE_8_CHAN  3
#define SCAN_SIZE_16_CHAN 4
#define SCAN_SIZE_32_CHAN 5
#define SCAN_SIZE_64_CHAN 6

/* scan clock source options */
#define SCAN_CLK_SRC_GEN_A           0
#define SCAN_CLK_SRC_GEN_B           1
#define SCAN_CLK_SRC_EXT_SYNC        2
#define SCAN_CLK_SRC_BCR_INPUT_SYNC  3

/* scan mode options */
#define SCAN_MODE_MULTIPLE 0
#define SCAN_MODE_SINGLE   1

/* single scan channel limits */
#define MIN_CHANNEL  0
#define MAX_CHANNEL 63

struct ai64ll_inputscan
{
	unsigned long   scan_size;		// Range: 0-6
	unsigned long   single_scan_channel;	// Range: 0-63
	char 		enable_scan;		// Range: 0-1
};

/*
 * IOCTL_AI64LL_GET_DEVICE_ERROR
 *
 * Parameter = unsigned long *
 *     RANGE: 0x20000000-0x20000008
 */

#define AI64LL_SUCCESS                    0x20000000
#define AI64LL_INVALID_PARAMETER          0x20000001
#define AI64LL_IOCTL_TIMEOUT              0x20000002
#define AI64LL_FAULT                      0x20000003

/*
 * IOCTL_AI64LL_READ_DEBUG
 *
 * Parameter = unsigned long *;
 */

#define	AI64LL_DEBUG_SIZE	16

/*
 * IOCTL_AI64LL_GET_DRIVER_INFO
 *
 * Parameter = ai64ll_driver_info_t*
 */

typedef struct
{
	char	version[8];
	char	built[32];
} ai64ll_driver_info_t;


#ifdef AI64LL_MMAP_SUPPORT   /* drd - MMAP SUPPORT */
/*
 * IOCTL_MMAP_SELECT
 *
 * MMAP Selection Commands 
 */
#define AI64LL_SELECT_GSC_MMAP      1         /* Select GSC Register */
#define AI64LL_SELECT_PLX_MMAP      2         /* Select PLX Register */
#define AI64LL_GSC_MMAP_SIZE        512       /* GSC MMAP Size */
#define AI64LL_PLX_MMAP_SIZE        256       /* PLX MMAP Size */
#endif /* end AI64LL_MMAP_SUPPORT */

#define	AI64LL_REGISTER_TYPE_MASK	0xC0000000L

/* Registers: GSC specific registers */
#define	AI64LL_GSC_REGISTER	0x00000000L
#define AI64LL_GSC_BCR	(AI64LL_GSC_REGISTER + 0)
#define AI64LL_GSC_ICR	(AI64LL_GSC_REGISTER + 1)
#define AI64LL_GSC_IBCR (AI64LL_GSC_REGISTER + 3)
#define AI64LL_GSC_CCNT	(AI64LL_GSC_REGISTER + 6)
#define AI64LL_GSC_SSCR	(AI64LL_GSC_REGISTER + 8)
#define AI64LL_GSC_FREV	(AI64LL_GSC_REGISTER + 10)
#define AI64LL_GSC_ACAL	(AI64LL_GSC_REGISTER + 11)

/* Access channel number 00 to 63 using the following define */
#define  AI64LL_GSC_CH(Num) (AI64LL_GSC_REGISTER + 64 + Num)


/* Register: PCI Configuration Registers */
#define	AI64LL_PCI_REGISTER		0x40000000L
#define AI64LL_PCI_IDR			(AI64LL_PCI_REGISTER + 0)
#define AI64LL_PCI_CR_SR			(AI64LL_PCI_REGISTER + 1)
#define AI64LL_PCI_REV_CCR		(AI64LL_PCI_REGISTER + 2)
#define AI64LL_PCI_CLSR_LTR_HTR_BISTR	(AI64LL_PCI_REGISTER + 3)
#define AI64LL_PCI_BAR0			(AI64LL_PCI_REGISTER + 4)
#define AI64LL_PCI_BAR1			(AI64LL_PCI_REGISTER + 5)
#define AI64LL_PCI_BAR2			(AI64LL_PCI_REGISTER + 6)
#define AI64LL_PCI_BAR3			(AI64LL_PCI_REGISTER + 7)
#define AI64LL_PCI_BAR4			(AI64LL_PCI_REGISTER + 8)
#define AI64LL_PCI_BAR5			(AI64LL_PCI_REGISTER + 9)
#define AI64LL_PCI_CIS			(AI64LL_PCI_REGISTER + 10)
#define AI64LL_PCI_SVID_SID		(AI64LL_PCI_REGISTER + 11)
#define AI64LL_PCI_ERBAR			(AI64LL_PCI_REGISTER + 12)
#define AI64LL_PCI_ILR_IPR_MGR_MLR	(AI64LL_PCI_REGISTER + 15)

/* Registers: PLX PCI9080: Local Configuration Registers */
#define	AI64LL_PLX_REGISTER	0x80000000L
#define AI64LL_PLX_LASORR	(AI64LL_PLX_REGISTER + 0)
#define AI64LL_PLX_LASORR	(AI64LL_PLX_REGISTER + 0)
#define AI64LL_PLX_LAS0BA	(AI64LL_PLX_REGISTER + 1)
#define AI64LL_PLX_MARBR		(AI64LL_PLX_REGISTER + 2)
#define AI64LL_PLX_BIGEND	(AI64LL_PLX_REGISTER + 3)
#define AI64LL_PLX_EROMRR	(AI64LL_PLX_REGISTER + 4)
#define AI64LL_PLX_EROMBA	(AI64LL_PLX_REGISTER + 5)
#define AI64LL_PLX_LBRD0		(AI64LL_PLX_REGISTER + 6)
#define AI64LL_PLX_DMRR		(AI64LL_PLX_REGISTER + 7)
#define AI64LL_PLX_DMLBAM	(AI64LL_PLX_REGISTER + 8)
#define AI64LL_PLX_DMLBAI	(AI64LL_PLX_REGISTER + 9)
#define AI64LL_PLX_DMPBAM	(AI64LL_PLX_REGISTER + 10)
#define AI64LL_PLX_DMCFGA	(AI64LL_PLX_REGISTER + 11)
#define AI64LL_PLX_LAS1RR	(AI64LL_PLX_REGISTER + 60) /* drd */
#define AI64LL_PLX_LAS1BA	(AI64LL_PLX_REGISTER + 61) /* drd */
#define AI64LL_PLX_LBRD1		(AI64LL_PLX_REGISTER + 62) /* drd */

/* Registers: PLX PCI9080: Runtime Registers */
#define AI64LL_PLX_MBOX0		(AI64LL_PLX_REGISTER + 16)
#define AI64LL_PLX_MBOX1		(AI64LL_PLX_REGISTER + 17)
#define AI64LL_PLX_MBOX2		(AI64LL_PLX_REGISTER + 18)
#define AI64LL_PLX_MBOX3		(AI64LL_PLX_REGISTER + 19)
#define AI64LL_PLX_MBOX4		(AI64LL_PLX_REGISTER + 20)
#define AI64LL_PLX_MBOX5		(AI64LL_PLX_REGISTER + 21)
#define AI64LL_PLX_MBOX6		(AI64LL_PLX_REGISTER + 22)
#define AI64LL_PLX_MBOX7		(AI64LL_PLX_REGISTER + 23)
#define AI64LL_PLX_P2LDBELL	(AI64LL_PLX_REGISTER + 24)
#define AI64LL_PLX_L2PDBELL	(AI64LL_PLX_REGISTER + 25)
#define AI64LL_PLX_INTCSR	(AI64LL_PLX_REGISTER + 26)
#define AI64LL_PLX_CNTRL		(AI64LL_PLX_REGISTER + 27)
#define AI64LL_PLX_PCIHIDR	(AI64LL_PLX_REGISTER + 28)
#define AI64LL_PLX_PCIHREV	(AI64LL_PLX_REGISTER + 29)
#define AI64LL_PLX_MBOX0_ALT	(AI64LL_PLX_REGISTER + 30)
#define AI64LL_PLX_MBOX1_ALT	(AI64LL_PLX_REGISTER + 31)

/* Registers: PLX PCI9080: DMA Registers */
#define AI64LL_PLX_DMAMODE0	(AI64LL_PLX_REGISTER + 32)
#define AI64LL_PLX_DMAPADR0	(AI64LL_PLX_REGISTER + 33)
#define AI64LL_PLX_DMALADR0	(AI64LL_PLX_REGISTER + 34)
#define AI64LL_PLX_DMASIZ0	(AI64LL_PLX_REGISTER + 35)
#define AI64LL_PLX_DMADPR0	(AI64LL_PLX_REGISTER + 36)
#define AI64LL_PLX_DMAMODE1	(AI64LL_PLX_REGISTER + 37)
#define AI64LL_PLX_DMAPADR1	(AI64LL_PLX_REGISTER + 38)
#define AI64LL_PLX_DMALADR1	(AI64LL_PLX_REGISTER + 39)
#define AI64LL_PLX_DMASIZ1	(AI64LL_PLX_REGISTER + 40)
#define AI64LL_PLX_DMADPR1	(AI64LL_PLX_REGISTER + 41)
#define AI64LL_PLX_DMACSR01	(AI64LL_PLX_REGISTER + 42)
#define AI64LL_PLX_DMAARB	(AI64LL_PLX_REGISTER + 43)
#define AI64LL_PLX_DMATHR	(AI64LL_PLX_REGISTER + 44)

/* Registers: PLX PCI9080: Messageing Queue Registers */
#define AI64LL_PLX_OPLFIS	(AI64LL_PLX_REGISTER + 12)
#define AI64LL_PLX_OPLFIM	(AI64LL_PLX_REGISTER + 13)
#define AI64LL_PLX_IQP		(AI64LL_PLX_REGISTER + 16)
#define AI64LL_PLX_OQP		(AI64LL_PLX_REGISTER + 17)
#define AI64LL_PLX_MQCR		(AI64LL_PLX_REGISTER + 48)
#define AI64LL_PLX_QBAR		(AI64LL_PLX_REGISTER + 49)
#define AI64LL_PLX_IFHPR	(AI64LL_PLX_REGISTER + 50)
#define AI64LL_PLX_IFTPR	(AI64LL_PLX_REGISTER + 51)
#define AI64LL_PLX_IPHPR	(AI64LL_PLX_REGISTER + 52)
#define AI64LL_PLX_IPTPR	(AI64LL_PLX_REGISTER + 53)
#define AI64LL_PLX_OFHPR	(AI64LL_PLX_REGISTER + 54)
#define AI64LL_PLX_OFTPR	(AI64LL_PLX_REGISTER + 55)
#define AI64LL_PLX_OPHPR	(AI64LL_PLX_REGISTER + 56)
#define AI64LL_PLX_OPTPR	(AI64LL_PLX_REGISTER + 57)
#define AI64LL_PLX_QSR		(AI64LL_PLX_REGISTER + 58)


/* These are the masks for the AI64LL Board Control Register. */
#define BC_AIM0_MASK            0x0001
#define BC_AIM1_MASK            0x0002
#define BC_AIM2_MASK            0x0004
#define BC_AIM_MASK             0x0007
#define BC_AIM_SHIFT            0
#define BC_RANGE0_MASK          0x0010
#define BC_RANGE1_MASK          0x0020
#define BC_RANGE_MASK           0x0030
#define BC_RANGE_SHIFT          4
#define BC_DATA_FORMAT_MASK     0x0040
#define BC_DATA_FORMAT_SHIFT    6
#define BC_INPUT_SYNC_MASK      0x1000
#define BC_INPUT_SYNC_SHIFT     12
#define BC_AUTO_CAL_MASK        0x2000
#define BC_AUTO_CAL_SHIFT       13
#define BC_AUTOCAL_PASS_MASK    0x4000
#define BC_AUTOCAL_PASS_SHIFT   14
#define BC_INITIALIZE_MASK      0x8000
#define BC_INITIALIZE_SHIFT     15

/* These are the masks for the AI64LL Interrupt Control Registers. */
#define IC_INT_EVENT_MASK            0x0077
#define IC_INT_OCC_MASK              0x0088
#define IC_DISABLE_ALL_INTS_VAL      0x0000

#define IC_IRQ0_MASK                 0x0008
#define IC_IRQ0_SHIFT                0
#define IC_IDLE_VAL                  0x0000
#define IC_AUTO_CAL_COMP_VAL         0x0001
#define IC_IRQ0_REQUEST_MASK         0x0008

#define IC_IRQ1_REQUEST_MASK         0x0080

/* These are the masks for the AI64LL Input Data Buffer Register. */
#define IDB_DATA_IN_MASK     0x0000FFFF
#define IDB_CHAN_00_TAG_MASK 0x00010000

/* These are the masks for the AI64LL Analog Input Data. */

/*** OFFSET BINARY FORMAT ***/
#define OB_16BIT_MAX_POSITIVE             0xFFFF
#define OB_16BIT_PLUS_ONE                 0x8001
#define OB_16BIT_ZERO                     0x8000
#define OB_16BIT_MINUS_ONE                0x7FFF
#define OB_16BIT_MAX_NEGATIVE             0x0000

#define OB_12BIT_MAX_POSITIVE             0x0FFF
#define OB_12BIT_PLUS_ONE                 0x0801
#define OB_12BIT_ZERO                     0x0800
#define OB_12BIT_MINUS_ONE                0x07FF
#define OB_12BIT_MAX_NEGATIVE             0x0000

/*** TWO'S COMPLEMENT FORMAT ***/
#define TC_16BIT_MAX_POSITIVE             0x7FFF
#define TC_16BIT_PLUS_ONE                 0x0001
#define TC_16BIT_ZERO                     0x0000
#define TC_16BIT_MINUS_ONE                0xFFFF
#define TC_16BIT_MAX_NEGATIVE             0x8000

#define TC_12BIT_MAX_POSITIVE             0x07FF
#define TC_12BIT_PLUS_ONE                 0x0001
#define TC_12BIT_ZERO                     0x0000
#define TC_12BIT_MINUS_ONE                0xFFFF
#define TC_12BIT_MAX_NEGATIVE             0xF800

/* These are the masks for the AI64LL Scan and Sync Control Register. */
#define SC_AIN_SCAN_SIZE_MASK       0x00000007
#define SC_AIN_SCAN_SIZE_SHIFT      0
#define SC_AIN_ENABLE_SCAN_MASK     0x00000020
#define SC_AIN_ENABLE_SCAN_SHIFT    5
#define SC_AIN_SINGLE_CHAN_MASK     0x0003F000
#define SC_AIN_SINGLE_CHAN_SHIFT    12
#define SC_AIN_RESET_INPUTS_MASK    0x00040000
#define SC_AIN_RESET_INPUTS_SHIFT   18

/* These are the masks for the PCI DMA Control Registers. */
#define DMA_CMD_STAT_DONE      0x10
#define DMA_CMD_STAT_CMD_MASK  0x07
#define DMA_CMD_STAT_INT_CLEAR 0x08
#define DMA_CMD_STAT_START     0x0003
#define DMA_CMD_STAT_ABORT     0x0004
#define DMA_MODE_DONE_ENABLE   0x00000400

#define START_DMA_CMD_0        0x0B
#define START_DMA_CMD_1        0xB00

/*** drd ***/
#define DMA_0_CSR_ENABLE         0x01	
#define DMA_0_CSR_START          0x02
#define DMA_0_CSR_ABORT          0x04
#define DMA_0_CSR_CLR_INT        0x08
#define DMA_0_CSR_DONE           0x10
/*** end drd ***/

#define STOP_DMA_CMD_0_MASK    0xFF01
#define STOP_DMA_CMD_1_MASK    0x00FF
#define DISABLE_DMA_CMD_0      0x0008
#define DISABLE_DMA_CMD_1      0x0800

#define NON_DEMAND_DMA_MODE    0x00020d43 /* 0x00020D43 */
#define DEMAND_DMA_MODE        0x00021D43

#define DMA_LOCAL_TO_PCI_BUS   0xA
#define DMA_PCI_TO_LOCAL_BUS   0x2

/***************** End Of Masks for registers ******************/

#endif /* __AI64LL_LINUX__ */

