// vim:ts=4 expandtab:
/*****************************************************************************
 *                                                                           *
 * File:         gsc16a0_ioctl.h                                             *
 *                                                                           *
 * Description:  The interface to the GSC16AO Linux device driver.           *
 *                                                                           *
 * Date:        05/20/2009                                                   *
 * History:                                                                  *
 *                                                                           *
 *   8  5/20/09 D. Dubash                                                    *
 *              new ioctl IOCTL_GSC16AO_SELECT_DIFFERENTIAL_SYNC_IO          *
 *              new ioctl IOCTL_GSC16AO_DISABLE_EXT_BURST_TRIGGER            *
 *                                                                           *
 *   7 05/05/09 D. Dubash                                                    *
 *              Added some defines                                           *
 *                                                                           *
 *   6 06/25/07 D. Dubash                                                    *
 *              Support for new GSC16AO16 sixteen channel board.             *
 *              Support for RH4.1.7 on 64_bit architecture                   *
 *                                                                           *
 *   5 11/16/06 D. Dubash                                                    *
 *              Support for redHawk 4.1.7.                                   *
 *                                                                           *
 *   4 12/07/05 D. Dubash                                                    *
 *              Added some #defines.                                         *
 *                                                                           *
 *   3  8/20/03 G. Barton                                                    *
 *              Add WAIT_FOR_INTERRUPT ioctl support                         *
 *                                                                           *
 *   2  5/30/03 G. Barton                                                    *
 *              Adapted for Redhawk Linux                                    *
 *                                                                           *
 *   1  2003    E. Hillman (evan@generalstandards.com)                       *
 *              Created                                                      *
 *                                                                           *
 *  Copyrights (c):                                                          *
 *      Concurrent Computer Corporation, 2003                                *
 *      General Standards Corporation (GSC), Dec 2002                        *
 *****************************************************************************/
/***
*** gsc16a0_ioctl.h 
***
***
***  General description of this file:
***  	Device driver source code for General Standards 16AO family of 
***  	16-bit analog output boards. This file is part of the Linux
***  	driver source distribution for this board.
***  	
***  Copyrights (c):
***  	General Standards Corporation (GSC), 2003
***
***  Author:
***  	Evan Hillman (evan@generalstandards.com)
***
***  Support:
***  	Primary support for this driver is provided by GSC. 
***
***  Platform (tested on, may work with others):
***  	Linux, kernel version 2.4.x, Red Hat distribution, Intel hardware.
***/

#ifndef _GSC16AO_IOCTL_H
#define _GSC16AO_IOCTL_H

#ifndef __KERNEL__
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#endif

/*** Registers: GSC Control and Data Registers ***/
typedef volatile struct {
	unsigned int	board_control;		/* Board Control Register (BCR) */
	unsigned int	channel_select;		/* Channel Select Register (CSR) */
	unsigned int	sample_rate;		/* Sample Rate Register (SRR) */
	unsigned int	buffer_ops;		/* Buffer Operations Register (BOR) */
	unsigned int	firmware_ops;		/* firmware and options - GSC16AO16 */
	unsigned int	reserved2;		/* Not used */
	unsigned int	output_data_buffer;	/* Output Data Buffer (ODB) */
	unsigned int	adjustable_clk;		/* Adjustable Clock Register (ACLK) */
} gsc16ao_gscregs;

#define MAX_BOARDS 16 /* maximum number of boards that driver supports. Arbitrarily chosen,
change if you can actully support more. */

/*** Board Specific Defines ***/
#define GSC16AO12_MAX_CHANNELS            12
#define GSC16AO12_MASTER_CLOCK            30000000        /* 30MHz */
#define GSC16AO12_MIN_SAMPLE_FREQ         458             /* Samples/sec */
#define GSC16AO12_MAX_SAMPLE_FREQ         400000          /* 400K Samples/sec */

#define GSC16AO12_DBL_MASTER_CLOCK        30000000.0      /* 30MHz */
#define GSC16AO12_DBL_MIN_SAMPLE_FREQ     457.77          /* Samples/sec */
#define GSC16AO12_DBL_MAX_SAMPLE_FREQ     400000.0        /* 400K Samples/sec */

#define GSC16AO16_MAX_CHANNELS            16
#define GSC16AO16_MASTER_CLOCK            45000000        /* 45MHz */
#define GSC16AO16_MIN_SAMPLE_FREQ         172             /* Samples/sec */
#define GSC16AO16_MAX_SAMPLE_FREQ         450000          /* 450K Samples/sec */

#define GSC16AO16_DBL_MASTER_CLOCK        45000000.0      /* 45MHz */
#define GSC16AO16_DBL_MIN_SAMPLE_FREQ     171.662         /* Samples/sec */
#define GSC16AO16_DBL_MAX_SAMPLE_FREQ     450000.0        /* 450K Samples/sec */

#define GSC16AO16_FLV_MAX_CHANNELS          16
#define GSC16AO16_FLV_MASTER_CLOCK          22932000        /* 22.9MHz */
#define GSC16AO16_FLV_MIN_SAMPLE_FREQ       87              /* Samples/sec */
#define GSC16AO16_FLV_MAX_SAMPLE_FREQ       450000          /* 450K Samples/sec */

#define GSC16AO16_FLV_DBL_MASTER_CLOCK      22932000.0      /* 22.9MHz */
#define GSC16AO16_FLV_DBL_MIN_SAMPLE_FREQ   87.479          /* Samples/sec */
#define GSC16AO16_FLV_DBL_MAX_SAMPLE_FREQ   450000.0        /* 450K Samples/sec */

#define GSC16AO_MAX_CHANNELS              GSC16AO16_MAX_CHANNELS
/******************************/

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

/*
 * mmap offsets and sizes
 */
#define GSC16AO_GSC_REGS_MMAP_OFFSET   0          /* Select GSC registers */
#define GSC16AO_GSC_REGS_MMAP_SIZE     PAGE_SIZE  /* GSC regs mmap size */
#define GSC16AO_PLX_REGS_MMAP_OFFSET   PAGE_SIZE  /* Select PLX registers */
#define GSC16AO_PLX_REGS_MMAP_SIZE     PAGE_SIZE  /* PLX regs mmap size */

/*
 *	error codes.
 */
#define GSC16AO_SUCCESS                             0
#define GSC16AO_ERR_INVALID_PARAMETER               1
#define GSC16AO_ERR_INVALID_BUFFER_SIZE             2
#define GSC16AO_ERR_PIO_TIMEOUT                     3
#define GSC16AO_ERR_DMA_TIMEOUT                     4
#define GSC16AO_ERR_IOCTL_TIMEOUT                   5
#define GSC16AO_ERR_OPERATION_CANCELLED             6
#define GSC16AO_ERR_RESOURCE_ALLOCATION             7
#define GSC16AO_ERR_INVALID_REQUEST                 8
#define GSC16AO_ERR_AUTOCAL_FAILED    	            9

/*
 *	IOCTL defines
 */
#define IOCTL_GSC16AO                         's'

#define IOCTL_GSC16AO_NO_COMMAND                _IO(IOCTL_GSC16AO, 1)
#define IOCTL_GSC16AO_INIT_BOARD                _IO(IOCTL_GSC16AO, 2)  
#define IOCTL_GSC16AO_READ_REGISTER             _IOWR(IOCTL_GSC16AO, 3, struct register_params)   
#define IOCTL_GSC16AO_WRITE_REGISTER            _IOWR(IOCTL_GSC16AO, 4, struct register_params)
#define IOCTL_GSC16AO_SET_DEBUG_FLAGS           _IOW(IOCTL_GSC16AO, 5, struct gsc_debug_flags)
/* #define IOCTL_GSC16AO_REG_FOR_INT_NOTIFY        _IOW(IOCTL_GSC16AO, 6, unsigned long) */
#define IOCTL_GSC16AO_GET_DEVICE_ERROR          _IOR(IOCTL_GSC16AO, 7, unsigned long)
#define IOCTL_GSC16AO_SET_WRITE_MODE            _IOW(IOCTL_GSC16AO, 8, unsigned long)
#define IOCTL_GSC16AO_AUTO_CAL                  _IOW(IOCTL_GSC16AO, 9, unsigned long)
#define IOCTL_GSC16AO_WAIT_FOR_INTERRUPT        _IOW(IOCTL_GSC16AO, 10, unsigned long)
#define IOCTL_GSC16AO_PROGRAM_RATE_GEN          _IOW(IOCTL_GSC16AO, 11, unsigned long)
#define IOCTL_GSC16AO_SELECT_ACTIVE_CHAN        _IOW(IOCTL_GSC16AO, 12, struct chan_select)
#define IOCTL_GSC16AO_SET_OUT_BUFFER_SIZE       _IOW(IOCTL_GSC16AO, 13, unsigned long)
#define IOCTL_GSC16AO_GET_BUF_STATUS            _IOR(IOCTL_GSC16AO, 14, unsigned long)
#define IOCTL_GSC16AO_ENABLE_CLK                _IO(IOCTL_GSC16AO, 15)
#define IOCTL_GSC16AO_DISABLE_CLK               _IO(IOCTL_GSC16AO, 16)
#define IOCTL_GSC16AO_GET_CALIB_STATUS          _IOR(IOCTL_GSC16AO, 17,unsigned long)
#define IOCTL_GSC16AO_SELECT_DATA_FORMAT        _IOW(IOCTL_GSC16AO, 18, unsigned long)
#define IOCTL_GSC16AO_SELECT_SAMPLING_MODE      _IOW(IOCTL_GSC16AO, 19,unsigned long)
#define IOCTL_GSC16AO_GET_BURSTING_STATUS       _IOW(IOCTL_GSC16AO, 20,unsigned long)
#define IOCTL_GSC16AO_BURST_TRIGGER             _IO(IOCTL_GSC16AO, 21)
#define IOCTL_GSC16AO_ENABLE_REMOTE_GND_SENSE   _IO(IOCTL_GSC16AO, 22)
#define IOCTL_GSC16AO_DISABLE_REMOTE_GND_SENSE  _IO(IOCTL_GSC16AO, 23)
#define IOCTL_GSC16AO_SELECT_OUT_CLKING_MODE    _IOW(IOCTL_GSC16AO, 24,unsigned long)
#define IOCTL_GSC16AO_SELECT_CLK_SOURCE         _IOW(IOCTL_GSC16AO, 25, unsigned long)
#define IOCTL_GSC16AO_GET_CLK_STATUS            _IOR(IOCTL_GSC16AO, 26, unsigned long)
#define IOCTL_GSC16AO_SINGLE_OUTPUT_CLK_EVT     _IO(IOCTL_GSC16AO, 27)
#define IOCTL_GSC16AO_SELECT_BUF_CONFIG         _IOW(IOCTL_GSC16AO, 28,unsigned long)
#define IOCTL_GSC16AO_LOAD_ACCESS_REQ           _IO(IOCTL_GSC16AO, 29)
#define IOCTL_GSC16AO_GET_CIR_BUF_STATUS        _IOR(IOCTL_GSC16AO, 30,unsigned long)
#define IOCTL_GSC16AO_CLEAR_BUFFER              _IO(IOCTL_GSC16AO, 31)
#define IOCTL_GSC16AO_GET_DEVICE_TYPE           _IOR(IOCTL_GSC16AO, 32,unsigned long)
#define IOCTL_GSC16AO_SET_TIMEOUT               _IOW(IOCTL_GSC16AO, 33, unsigned long)
#define IOCTL_GSC16AO_ENABLE_PCI_INTERRUPTS     _IO(IOCTL_GSC16AO, 34)
#define IOCTL_GSC16AO_DISABLE_PCI_INTERRUPTS    _IO(IOCTL_GSC16AO, 35)
#define IOCTL_GSC16AO_GET_OFFSET                _IOW(IOCTL_GSC16AO, 36, unsigned long)
#define IOCTL_GSC16AO_GET_BOARD_INFO            _IOWR(IOCTL_GSC16AO, 37, board_info_t)   
#define IOCTL_GSC16AO_SELECT_DIFFERENTIAL_SYNC_IO _IOW(IOCTL_GSC16AO, 38, unsigned long)
#define IOCTL_GSC16AO_DISABLE_EXT_BURST_TRIGGER  _IOW(IOCTL_GSC16AO, 39, unsigned long)
#define IOCTL_GSC16AO_SELECT_OUTPUT_RANGE        _IOW(IOCTL_GSC16AO, 40, unsigned long)
#define IOCTL_GSC16AO_SELECT_OUTPUT_FILTER       _IOW(IOCTL_GSC16AO, 41, unsigned long)


/*************************************************************************
** IOCTL_GSC16AO_NO_COMMAND
**
** Parameter = NONE
**/

/*************************************************************************
**  IOCTL_GSC16AO_INIT_BOARD
**
**  Parameter = NONE
**/

/*************************************************************************
** IOCTL_GSC16AO_READ_REGISTER	  
** IOCTL_GSC16AO_WRITE_REGISTER	  
**
** Parameter = REGISTER_PARAMS *pRegParams;
**
** This structure is used to store information about a register. The
** IOCTL_GSC16AO_READ_REGISTER and IOCTL_GSC16AO_WRITE_REGISTER ioctl commands use this
** structure to read a particular register and to write a value into a particular
** register.
**   regset field identifies the specific register set
**   regnum holds the index of the register
**   regval is register value that is to be written or read
**/
typedef struct register_params {
	unsigned short regset;
	unsigned short regnum;
	unsigned long  regval;
} REGISTER_PARAMS, *PREGISTER_PARAMS;

typedef struct board_info {
    unsigned int    max_channels;
    unsigned int    master_clock;
    unsigned int    min_sample_freq;
    unsigned int    max_sample_freq;

    double          dbl_master_clock;       /* double version of same */
    double          dbl_min_sample_freq;    /* double version of same */
    double          dbl_max_sample_freq;    /* double version of same */

    unsigned int    firmware_ops;
    unsigned int    board_type;
    unsigned int    flv;                    /* gsc16ao_flv board */
    unsigned int    differential;           /* differential/single-ended */
    unsigned int    high_level;             /* 1=high_level, 0=high_current */
    char            filter[15];
    char            board_name[12];
} board_info_t;

/* Voltage setting for GSC16AO16 - MAKE OBSOLETE AS THESE CONFLICT
 * WITH GSC16AO16_FLV BOARD. NOW USE IOCTL's INSTEAD OF MANUPULATING
 * REGISTERS DIRECTLY */
#if 0
#define GSC16AO16_1_25V   (0 << 16)
#define GSC16AO16_2_5V    (1 << 16)
#define GSC16AO16_5V      (2 << 16)
#define GSC16AO16_10V     (3 << 16)
#endif

/* Voltage setting for GSC16AO16 and GSC16AO16_FLV */
#define GSC16AO16_RANGE_1_25     (1)
#define GSC16AO16_RANGE_1_5      (2)
#define GSC16AO16_RANGE_2_5      (3)
#define GSC16AO16_RANGE_5        (4)
#define GSC16AO16_RANGE_10       (5)

/* Output Filter Range (FLV only) */
#define GSC16AO16_FILTER_NONE   (1)
#define GSC16AO16_FILTER_A      (2)
#define GSC16AO16_FILTER_B      (3)

/* Register sets */
#define	GSC16AO_GSC_REGISTER	(0)
#define	GSC16AO_PCI_REGISTER	(1)
#define	GSC16AO_PLX_REGISTER	(2)

/* GSC registers */
#define GSC16AO_GSC_BCR		(0)
#define GSC16AO_GSC_CSR		(1)
#define GSC16AO_GSC_SRR		(2)
#define GSC16AO_GSC_BOR		(3)
#define GSC16AO_GSC_REV		(4)
#define GSC16AO_GSC_ODBR	(6)
#define GSC16AO_GSC_ACLK	(7)

/* PCI Configuration Registers */
#define GSC16AO_PCI_IDR			(0)
#define GSC16AO_PCI_CR_SR		(1)
#define GSC16AO_PCI_REV_CCR		(2)
#define GSC16AO_PCI_CLSR_LTR_HTR_BISTR	(3)
#define GSC16AO_PCI_BAR0		(4)
#define GSC16AO_PCI_BAR1		(5)
#define GSC16AO_PCI_BAR2		(6)
#define GSC16AO_PCI_BAR3		(7)
#define GSC16AO_PCI_BAR4		(8)
#define GSC16AO_PCI_BAR5		(9)
#define GSC16AO_PCI_CIS			(10)
#define GSC16AO_PCI_SVID_SID		(11)
#define GSC16AO_PCI_ERBAR		(12)
#define GSC16AO_PCI_ILR_IPR_MGR_MLR	(15)

/* PLX PCI9080: Local Configuration Registers */
#define GSC16AO_PLX_LASORR	(0)
#define GSC16AO_PLX_LASORR	(0)
#define GSC16AO_PLX_LAS0BA	(1)
#define GSC16AO_PLX_MARBR	(2)
#define GSC16AO_PLX_BIGEND	(3)
#define GSC16AO_PLX_EROMRR	(4)
#define GSC16AO_PLX_EROMBA	(5)
#define GSC16AO_PLX_LBRD0	(6)
#define GSC16AO_PLX_DMRR	(7)
#define GSC16AO_PLX_DMLBAM	(8)
#define GSC16AO_PLX_DMLBAI	(9)
#define GSC16AO_PLX_DMPBAM	(10)
#define GSC16AO_PLX_DMCFGA	(11)
#define GSC16AO_PLX_LAS1RR	(60)
#define GSC16AO_PLX_LAS1BA	(61)
#define GSC16AO_PLX_LBRD1	(62)

/* Registers: PLX PCI9080: Runtime Registers */
#define GSC16AO_PLX_MBOX0	(16)
#define GSC16AO_PLX_MBOX1	(17)
#define GSC16AO_PLX_MBOX2	(18)
#define GSC16AO_PLX_MBOX3	(19)
#define GSC16AO_PLX_MBOX4	(20)
#define GSC16AO_PLX_MBOX5	(21)
#define GSC16AO_PLX_MBOX6	(22)
#define GSC16AO_PLX_MBOX7	(23)
#define GSC16AO_PLX_P2LDBELL	(24)
#define GSC16AO_PLX_L2PDBELL	(25)
#define GSC16AO_PLX_INTCSR	(26)
#define GSC16AO_PLX_CNTRL	(27)
#define GSC16AO_PLX_PCIHIDR	(28)
#define GSC16AO_PLX_PCIHREV	(29)
#define GSC16AO_PLX_MBOX0_ALT	(30)
#define GSC16AO_PLX_MBOX1_ALT	(31)

/* Registers: PLX PCI9080: DMA Registers */
#define GSC16AO_PLX_DMAMODE0	(32)
#define GSC16AO_PLX_DMAPADR0	(33)
#define GSC16AO_PLX_DMALADR0	(34)
#define GSC16AO_PLX_DMASIZ0	(35)
#define GSC16AO_PLX_DMADPR0	(36)
#define GSC16AO_PLX_DMAMODE1	(37)
#define GSC16AO_PLX_DMAPADR1	(38)
#define GSC16AO_PLX_DMALADR1	(39)
#define GSC16AO_PLX_DMASIZ1	(40)
#define GSC16AO_PLX_DMADPR1	(41)
#define GSC16AO_PLX_DMACSR01	(42)
#define GSC16AO_PLX_DMAARB	(43)
#define GSC16AO_PLX_DMATHR	(44)

/* Registers: PLX PCI9080: Messaging Queue Registers */
#define GSC16AO_PLX_OPLFIS	(12)
#define GSC16AO_PLX_OPLFIM	(13)
#define GSC16AO_PLX_IQP		(16)
#define GSC16AO_PLX_OQP		(17)
#define GSC16AO_PLX_MQCR	(48)
#define GSC16AO_PLX_QBAR	(49)
#define GSC16AO_PLX_IFHPR	(50)
#define GSC16AO_PLX_IFTPR	(51)
#define GSC16AO_PLX_IPHPR	(52)
#define GSC16AO_PLX_IPTPR	(53)
#define GSC16AO_PLX_OFHPR	(54)
#define GSC16AO_PLX_OFTPR	(55)
#define GSC16AO_PLX_OPHPR	(56)
#define GSC16AO_PLX_OPTPR	(57)

/*************************************************************************
** IOCTL_GSC16AO_SET_DEBUG_FLAGS	  
**
** Parameter = GSC_DEBUG_FLAGS *pFlags;
**
** This structure is used to chnage drivers internal debug flags
**   db_class holds bit mask of enabled debug classes
**   db_level holds the debug level value
**/
typedef struct gsc_debug_flags {
	unsigned long db_classes;
	unsigned long db_level;
} GSC_DEBUG_FLAGS, *PGSC_DEBUG_FLAGS;

/*************************************************************************
** IOCTL_GSC16AO_WAIT_FOR_INTERRUPT
**
** Wait for an interrupt signaling specified condition is true 
**
** Parameter = unsigned long irqEvent;
**     see codes below
**/
#define GSC16AO_BCR_IRQ_OUT_BUFFER_EMPTY        (2 << 8)
#define GSC16AO_BCR_IRQ_OUT_BUFFER_LOW_QUARTER  (3 << 8)
#define GSC16AO_BCR_IRQ_OUT_BUFFER_HIGH_QUARTER (4 << 8)
#define GSC16AO_BCR_IRQ_BURST_TRIGGER_READY     (5 << 8)
#define GSC16AO_BCR_IRQ_LOAD_READY              (6 << 8)

/* Board Control Registers */
#define GSC16AO_BCR_BURST_ENABLED            	(1<<0)
#define GSC16AO_BCR_BURST_READY              	(1<<1)
#define GSC16AO_BCR_BURST_TRIGGER            	(1<<2)
#define GSC16AO_BCR_REMOTE_GROUND_SENSE      	(1<<3)
#define GSC16AO_BCR_OFFSET_BINARY            	(1<<4)
#define GSC16AO_BCR_SIMULTANEOUS_OUTPUTS     	(1<<7)

/* NO LONGER NEEDED */
#if 0
/*************************************************************************
**  IOCTL_GSC16AO_GET_DEVICE_ERROR
**
** returns the last error code.  

**  Parameter = unsigned long *pError
**
** Possible error codes are:
**/
	enum {
		GSC16AO_NO_ERR,
		GSC16AO_INVALID_PARAMETER_ERR,
		GSC16AO_RESOURCE_ERR,
		GSC16AO_BOARD_ACCESS_ERR,
		GSC16AO_DEVICE_ADD_ERR,
		GSC16AO_ALREADY_OPEN_ERR,
		GSC16AO_INVALID_BOARD_STATUS_ERR,
		GSC16AO_FIFO_BUFFER_ERR
	};
#endif

/*************************************************************************
**  IOCTL_GSC16AO_WRITE_MODE_CONFIG
**
** select PIO or DMA writes to the hardware
**
**  Parameter = unsigned long *pWriteMode
**
** possible values are:
**/
	enum
	{
		GSC16AO_SCAN_MODE,
		GSC16AO_DMA_MODE
	};

/*************************************************************************
** IOCTL_GSC16AO_AUTO_CAL
**
** Parameter = none
**
**/

/*************************************************************************
** IOCTL_GSC16AO_INT_SOURCE
**
** select the interrupt source for notify.
**
** Parameter = unsigned long *pIntSource;
** 
**
** Possible values are:
**/
#define	GSC16AO_INT_IDLE               0
#define GSC16AO_INT_CAL_COMPLETE       1
#define GSC16AO_INT_OUTPUT_EMPTY       2
#define GSC16AO_INT_OUTPUT_LOW_QTR     3
#define GSC16AO_INT_OUTPUT_HITH_QTR    4
#define GSC16AO_INT_OUTPUT_BURST_READY 5
#define GSC16AO_INT_LOAD_READY         6
#define GSC16AO_INT_END_LOAD_READY     7

/*************************************************************************
** IOCTL_GSC16AO_PROGRAM_RATE_GEN
**
** Set the divisor value for the rate generator.  The frequency at which
** the channels are scanned and sampled is :
**
** Frequency (Hz) = 30,000,000/nrate
**
** Parameter = unsigned long *nrate;
** Range: 0-0xFFFF //BUGBUG
**/
//#define GSC16AO_MAX_RATE_GEN 0xffff
#define GSC16AO_MAX_RATE_GEN 0x3ffff

/*************************************************************************
** IOCTL_GSC16AO_SELECT_ACTIVE_CHAN
**
** Activate a channel group and select the number of channels in the group.
**
** Parameter = CHAN_SELECT chans;
** 
**/

typedef struct chan_select {
	unsigned long ulChannels;
	unsigned long ulNumChannels;
} CHAN_SELECT,  *PCHAN_SELECT;

/*************************************************************************
** IOCTL_GSC16AO_SET_OUT_BUFFER_SIZE
**
**
** Set the size of the output buffer in bytes. 
** Parameter = ulong *ulSize;
**
**  Possible values are:

** GSC16AO12 
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

** GSC16AO16 
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
**/

/*************************************************************************
** IOCTL_GSC16AO_GET_BUF_STATUS
**
** Get the current output buffer status
**
** Parameter = unsigned long *status;
**
** The return value is one of the following:
**/

#define	  GSC16AO_OUTPUT_EMPTY    (1<<12)
#define	  GSC16AO_OUTPUT_LOW_QTR  (1<<13)
#define	  GSC16AO_OUTPUT_HIGH_QTR (1<<14)
#define	  GSC16AO_OUTPUT_FULL     (1<<15)
#define	  GSC16AO_OUTPUT_BUFFER_OVERFLOW  (1<<16)
#define	  GSC16AO_OUTPUT_FRAME_OVERFLOW  (1<<17)
#define   GSC16AO_OUTPUT_ERROR (GSC16AO_OUTPUT_FULL |            \
                                GSC16AO_OUTPUT_BUFFER_OVERFLOW | \
                                GSC16AO_OUTPUT_FRAME_OVERFLOW)
#define   GSC16AO_BUFFER_STATUS_MASK (0x0f<<12)

/*************************************************************************
** IOCTL_GSC16AO_ENABLE_CLK
**
** Enable output clocking.
**
** Parameter = not used;
**/

/*************************************************************************
** IOCTL_GSC16AO_DISABLE_CLK
**
** Disable output clocking.
**
** Parameter = not used;
**/

/*************************************************************************
** IOCTL_GSC16AO_CLEAR_INT_REQUEST
**
** clear the interrupt request at the PLX
**
** Parameter = none;
**
**/

/*************************************************************************
** IOCTL_GSC16AO_GET_CALIB_STATUS
**
** get the results of the last calibration operation.
**
** Parameter = unsigned long *pulStatus;
** 
** Possible return values are:
**/
enum
{
	AUTOCAL_FAILED,
	AUTOCAL_PASSED
};

/*************************************************************************
** IOCTL_GSC16AO_SELECT_DATA_FORMAT
**
** Parameter = int *pulFormat;
** 
** Possible Values are:
**/
#define GSC16AO_TWOS_COMP 0
#define	GSC16AO_OFFSET_BINARY 1 

/*************************************************************************
** IOCTL_GSC16AO_SELECT_SAMPLING_MODE
**
** Parameter: long *ulMode
** 
** Possible values are:
**
**/

#define GSC16AO_CONT_MODE 0
#define GSC16AO_BURST_MODE 1

/*************************************************************************
** IOCTL_GSC16AO_SELECT_DIFFERENTIAL_SYNC_IO
**
** Parameter: long *ulMode
** 
** Possible values are:
**
**/

#define GSC16AO_TTL        0    /* TTL signaling */
#define GSC16AO_LVDS       1    /* low voltage differential signaling */

/*************************************************************************
** IOCTL_GSC16AO_DISABLE_EXT_BURST_TRIGGER
**
** Parameter: long *ulMode
** 
** Possible values are:
**
**/

#define GSC16AO_DISABLE_EXT_TRIG 0    /* disable external trigger */
#define GSC16AO_ENABLE_EXT_TRIG  1    /* enable external trigger */

/*************************************************************************
** IOCTL_GSC16AO_GET_BURSTING_STATUS
**
** Parameter: long *ulStatus
** 
** Possible values are:
**
**/

#define GSC16AO_BURST_NOT_READY 0
#define	GSC16AO_BURST_READY GSC16AO_BCR_BURST_READY

/*************************************************************************
** IOCTL_GSC16AO_BURST_TRIGGER
**
** Start a transfer of data from the output buffer to the selected
** active channels.
**
** Parameter = none;
**
**/

/*************************************************************************
** IOCTL_GSC16AO_ENABLE_REMOTE_GND_SENSE
**
** Set the hardware to use remote ground sense mode.
**
** Parameter = none;
**
**/

/*************************************************************************
** IOCTL_GSC16AO_DISABLE_REMOTE_GND_SENSE
**
** Set the hardware to use on-board ground reference.
**
** Parameter = none;
**
**/

/*************************************************************************
** IOCTL_GSC16AO_SELECT_OUT_CLKING_MODE
**
** Parameter: long *ulMode
** 
** Possible values are:
**
**/
enum {
	SEQUENTIAL,
	SIMULTANEOUS
};

/*************************************************************************
** IOCTL_GSC16AO_SELECT_CLK_SOURCE
**
** Parameter: long *ulMode
** 
** Possible values are:
**
**/
enum {
	INTERNAL,
	EXTERNAL
};

/*************************************************************************
** IOCTL_GSC16AO_GET_CLK_STATUS
**
** Parameter: long *ulMode
** 
**
** returns one of the following:
**/

#define	GSC16AO_CLOCK_NOT_READY 0
#define	GSC16AO_CLOCK_READY BOR_CLOCK_READY

/*************************************************************************
** IOCTL_GSC16AO_SINGLE_OUTPUT_CLK_EVENT
**
** Generate a clock strobe.
**
** Parameter = none;
**
**/

/*************************************************************************
** IOCTL_GSC16AO_SELECT_BUF_CONFIG
**
** Parameter: long *ulConfig
** 
** Possible values are:
**
**/

#define	GSC16AO_OPEN_BUF 0
#define GSC16AO_CIRCULAR_BUF (1<<8)

/*************************************************************************
** IOCTL_GSC16AO_LOAD_ACCESS_REQ
**
** Request access to the circular (closed) buffer.
**
** Parameter = none;
**
**/

/*************************************************************************
** IOCTL_GSC16AO_GET_CIR_BUF_STATUS
**
** Parameter: long *ulStatus
** 
** Possible values are:
**
**/

#define	GSC16AO_CIR_BUF_NOT_READY 0
#define	GSC16AO_CIR_BUF_READY BOR_LOAD_READY

/*************************************************************************
** IOCTL_GSC16AO_CLEAR_BUFFER
**
** Request access to the circular (closed) buffer.
**
** Parameter = none;
**
**/

/*************************************************************************
** IOCTL_GSC16AO_GET_DEVICE_TYPE
**
** Returns type of board found
**
** Parameter = unsigned long *pType;
**
**/
enum
{
	GSC_16AO_16,
	GSC_16AO_12,
	GSC_16AO_2
};

#endif	/* entire file */

