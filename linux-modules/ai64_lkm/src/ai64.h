// vim:ts=4 expandtab: 
/******************************************************************************
 *                                                                            *
 * File:        ai64.h                                                        *
 *                                                                            *
 * Description: The interface to the AI64 Linux device driver.                *
 *                                                                            *
 * Date:        01/07/2014                                                    *
 * History:                                                                   *
 *                                                                            *
 *  31  01/07/2014 D. Dubash                                                  *
 *      - if the number of samples was too small, the driver was faulting     *
 *        when a add_timer was still pending and the user attemped to perform *
 *        a device initialization or auto cal. The fix is to make sure        *
 *        an active add_timer is deleted if a timeout has not occurred.       *
 *      - previously, DEMAND DMA was behaving same and regular DMA, i.e.      *
 *        driver would block until sufficient samples are available. Now,     *
 *        DEMAND DMA lets the hardware perform the necessary flow control.    *
 *                                                                            *
 *  30  09/04/2013 D. Dubash                                                  * 
 *      General Standards is now delivering cards without I/O BAR1 present.   *
 *      Driver was failing the load. Now, the driver ignores any BAR that     *
 *      has zero address.                                                     *
 *                                                                            *
 *  29  04/03/2013 D. Higgins                                                 * 
 *      Support for RedHawk 6.3                                               *
 *                                                                            *
 *  28  08/22/2012 D. Dubash                                                  * 
 *      When running SimWB card 0 of three cards was getting an autocal       *
 *      timeout. It appears that for some reason, when other boards are       *
 *      going through their initialization and autocal, the first card        *
 *      gets an autocal complete, however, without any local interrupt bit    *
 *      set. What is done is that the KLOOGE is skipped if autocal_pending    *
 *      is set, i.e. we are in the middle of an autocal.                      *
 *                                                                            *
 *  27  06/01/2012 D. Dubash                                                  * 
 *      Support for RedHawk 6.0.4                                             *
 *                                                                            *
 *  26  01/27/2012 D. Dubash                                                  * 
 *      Support for RedHawk 6.0.2                                             *
 *                                                                            *
 *  25  01/12/2012 D. Dubash                                                  * 
 *      Release for newer 5.4.x variants (up to 5.4.13)                       *
 *                                                                            *
 *  24  10/11/2011 D. Dubash                                                  * 
 *      Support for new AI64SSA/C card                                        *
 *                                                                            *
 *  23  09/30/2011 D. Dubash                                                  * 
 *      Release for newer 5.4.x variants (up to 5.4.12)                       *
 *                                                                            *
 *  22  03/29/2011 D. Dubash                                                  * 
 *      Fixed a problem in the driver                                         *
 *                                                                            *
 *  21  11/17/2010 D. Dubash                                                  * 
 *      Support for RedHawk 5.4.6. Also fixed ai64_analog_in.c test           *
 *                                                                            *
 *  20  05/11/2010 D. Dubash                                                  * 
 *      Support for RedHawk 5.4.x                                             *
 *                                                                            *
 *  19  06/26/2009 D. Dubash                                                  * 
 *      Fixed AI64LL/AI64 detection method as it was failing detection of     *
 *      an AI64 card because the card had a non-zero conversion value in it.  *
 *                                                                            *
 *  18  02/24/2009 D. Dubash                                                  * 
 *      Support for RedHawk 5.2.x                                             *
 *                                                                            *
 *  17  03/25/2008 D. Dubash                                                  * 
 *      Support for RedHawk 5.1                                               *
 *                                                                            *
 *  16  03/19/2008 D. Dubash                                                  * 
 *      Support for RedHawk 4.2.3                                             *
 *                                                                            *
 *  15  01/29/2008 D. Dubash                                                  * 
 *      Support for RedHawk 4.2.2                                             *
 *                                                                            *
 *  14  06/19/2007 D. Dubash                                                  * 
 *      Fixed problem with 64bit Kernel DMA (added GFP_DMA32 flag)            *
 *                                                                            *
 *  13  03/21/2007 D. Dubash                                                  * 
 *      Add support for rpm.                                                  *
 *                                                                            *
 *  12  03/01/2007 D. Dubash                                                  * 
 *      Use converter counter to distinguish between AI64 and AI64LL cards.   *
 *                                                                            *
 *  11  01/11/07  D. Dubash                                                   *
 *      Change version to 4.1.7_1.                                            *
 *                                                                            *
 *  10  11/16/06  D. Dubash                                                   *
 *      Support for redhawk 4.1.7.                                            *
 *                                                                            *
 *   9  04/25/06  D. Dubash                                                   *
 *      added new variable ioctl_processing.                                  *
 *                                                                            *
 *   8  03/23/06  D. Dubash                                                   *
 *      Change revision number. Support for RedHawk 4.1.                      *
 *                                                                            *
 *   7  02/06/06  D. Dubash                                                   *
 *      Change revision number. Support for several new IOCTLS.               *
 *                                                                            *
 *   6  12/20/05  D. Dubash                                                   *
 *      Get rid of sysdep.h file. Support for RH2.3.3. Also get rid of the    *
 *      PciBar structure along with local_addr and runtime_addr ioremaps      *
 *      (replaced with local_reg_address and config_reg_address respectively) *
 *                                                                            *
 *   5  08/22/05  D. Dubash                                                   *
 *      Support for user allocation of DMA buffer.                            *
 *                                                                            *
 *   4  10/04/04  D. Dubash                                                   *
 *      Added new IOCTL_DEVICE_SET_POLAR, IOCTL_AI64_VALIDATE_CHAN_0 and      *
 *      IOCTL_AI64_GET_BUFFER_HWM ioctls and new #defines.                    *
 *                                                                            *
 *   3  08/27/04   D. Dubash                                                  *
 *      Support for 2.2 and 64 bit Opteron architecture                       *
 *                                                                            *
 *   2  07/15/04   R. Calabro                                                 *
 *      Updated to run on RedHawk 1.4                                         *
 *                                                                            *
 *   1  Nov 2002   Evan Hillman                                               *
 *      This is the header used by AI64 driver.                               *
 *                                                                            *
 ******************************************************************************/

/***
*** pci16AI64.H 
***
***  General description of this file:
***  	Driver structure and macro definitions for General Standards PCI-16AI64
***  	64 channel PCI/PMC A/D board. This file is part of the Linux
***  	driver source distribution for this board.
***  	
***  	This file is not necessary to compile application programs, therefore 
***  	should NOT be included in binary only driver distributions.
***
***  Copyrights (c):
***  	General Standards Corporation (GSC), October 2002
***
***  Author:
***  	Evan Hillman, GSC Inc. (evan@generalStandards.com)
***
***  Support:
***  	Primary support for this driver is provided by GSC. 
***
***  Platform (tested on, may work with others):
***  	Linux, kernel version 2.2.x, Intel hardware.
***
***  Revision history:
***  	Oct  2002: first release
***/

#if   	 LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#define  LINUX_2_4
#elif 	 LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
#define  LINUX_2_2
#else
#define  LINUX_2_0
#endif

#ifndef TRUE
#define TRUE (1==1)
#endif
#ifndef FALSE
#define FALSE (!TRUE)
#endif

/*****************************************************************
 *   DO NOT CHANGE THESE NAMES AS THEY ARE SEARCHED BY build_rpm * 
 *****************************************************************/
#define AI64_SLERT_VERSION         "10.13_0"
#define AI64_RedHawk_VERSION       "6.3_2"
#define AI64_HARDWARE_DESC "General Standards PMC-AI64SS Analog Input"
/*************************************************************/

/* module name(s) */
#ifdef CONFIG_SUSE_KERNEL
#define AI64_VERSION  AI64_SLERT_VERSION
#else
#define AI64_VERSION  AI64_RedHawk_VERSION
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
/* if 64-bit architecture */
//#if (defined(__i386__) && defined(CONFIG_HIGHMEM) && defined(CONFIG_HIGHMEM64G)) || defined(__x86_64__) || defined (__ia64__) || defined(__mips64__) || (defined(__mips__) && defined(CONFIG_HIGHMEM) && defined(CONFIG_64BIT_PHYS_ADDR))
#if defined(__x86_64__) || defined (__ia64__) || defined(__mips64__) || (defined(__mips__) && defined(CONFIG_HIGHMEM) && defined(CONFIG_64BIT_PHYS_ADDR))
typedef u64	uintptr_t;
#else /* 32-bit architecture */
typedef u32 uintptr_t;
#endif
#endif

#ifdef VERIFY_WRITE
#define get_user_ret(val, from, retval) {		   \
  if(access_ok(VERIFY_READ, from, sizeof(*(from)))) {	   \
    __get_user(val, from);				   \
  }							   \
  else {						   \
    return(retval);					   \
  }							   \
}
#define put_user_ret(val, to, retval) { 		   \
  if(access_ok(VERIFY_WRITE, to, sizeof(*(to)))) {	   \
    __put_user(val, to);				   \
  }							   \
  else {						   \
    return(retval);					   \
  }							   \
 }
#else
#define get_user_ret(val, from, retval) {		   \
  if(access_ok(from, sizeof(*(from)))) {	   \
    __get_user(val, from);				   \
  }							   \
  else {						   \
    return(retval);					   \
  }							   \
}
#define put_user_ret(val, to, retval) { 		   \
  if(access_ok(to, sizeof(*(to)))) {	   \
    __put_user(val, to);				   \
  }							   \
  else {						   \
    return(retval);					   \
  }							   \
 }
#endif

#if   	 LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
struct timeval {
	__kernel_old_time_t	tv_sec;		/* seconds */
	__kernel_suseconds_t	tv_usec;	/* microseconds */
};
#endif

#include <linux/interrupt.h>

/* module name(s) */
#define AI64_DEVICE_NAME      "ai64" /* Must match name in makefile */
#define AI64_DRIVER_VERSION   AI64_VERSION

#define GSC_SUBVENDOR PCI_VENDOR_ID_PLX

#define PCI_NUM_BARS 6

#define MAX_BOARDS 10 /* maximum number of boards that driver supports. Arbitrarily chosen,
change if you can actully support more. */

// 12AI64

#define GSE_SUBDEVICE12 0x2406

// 16AI64

#define GSE_SUBDEVICE16 0x2407


// CCC Specific
#define LARGE_FIFO		     1
#define DEF_NUM_DMA_BUFFERS	 5


struct board_entry {
	int device;
	int subsystem_vendor;
	int subsystem_device;
	char name[40];
	int index;
};


#if !defined(U64)
typedef union _U64
{
	struct
	{
		u32  LowPart;
		u32  HighPart;
	}u;

	u64 QuadPart;
} U64;
#endif

enum BufferState{
		DmaFree,
		DmaInUse,
	    DmaValidData,
	    DmaOverflow
};

enum {
	CONFIG_REGION,		/* Configuration Register - Region 0 */
	IO_REGION,			/* IO Register            - Region 1 */
	LOCAL_REGION,		/* Local Register         - Region 2 */

	/****/
	AI64_MAX_REGION	/* Must be last entry in enum */
};

typedef	struct
{
	u32	address;
	u32	size;
	u32	flags;
} dev_region_t;

struct ai_dma {
	u32					*dmabufferpool;
	unsigned	long	dmaoffset;
	int					dmasamples;
	int					dmastart;
	int					bufferstate;
	int					fifo_read;
};

/* card descriptor structure */
struct ai_board {
	struct timer_list watchdog_timer;

	struct pci_dev *pdev;       	/* the kernel PCI device descriptor */
	struct	 ai_dma		*dma;
	u32		*dmabuffer;		/* Current DMA buffer pointer */
	int timeout;      	      		/* timeout flag */
	int timeout_seconds;			/* time to wait before timeout (in seconds) */
	int dmaState;      	      		/* DMA mode transfer enable flag */
	int dmaSampleSize;				/* Number of Samples to DMA in Continous Mode */
	int cbuffer_r;					/* Current read  DMA buffer number */
	int cbuffer_w;					/* Current write DMA buffer number */
	int busy;						/* device opened */
	int irqlevel; 					/* interrupt level */
	int minor;						/* device minor number */
	unsigned int bufferThreshold;
	int error;						/* SDI error code */
	int signalno; 					/* signal index if user requested notification */
	int signalev;       	      	/* signal the user if this event occurs */
	volatile char init_pending; 	/* flag set if init IRQ expected */
	volatile char autocal_pending; 	/* flag set if autocal IRQ expected */
	volatile char chready_pending; 	/* flag set if channels ready IRQ expected */
	volatile char lo_hi_pending;    /* flag set if waiting for low to high buffer transition */
	volatile char wakeup_dma_pending; /* flag set if waiting for dma wakeup pending */
	volatile char wakeup_lohi_pending;/* flag set if waiting for lo/hi wakeup pending */
	volatile char dmainprogress; 	/* flag set by intr when DMA is started */
	volatile char dmaoverflow; 		/* flag set by intr when DMA has overflowed in continuous mode */
#ifdef LINUX_2_4
	wait_queue_head_t ioctlwq;		/* queue of tasks sleeping in ioctl */
	wait_queue_head_t dmawq;		/* queue of tasks sleeping on DMA transfer completion */
	wait_queue_head_t lohiwq;		/* queue of tasks sleeping on LO_HI interrupt completion */
#else
	struct wait_queue *ioctlwq;     /* queue of tasks sleeping in ioctl */
	struct wait_queue *dmawq;      	/* queue of tasks sleeping on DMA transfer completion */
	struct wait_queue *lohiwq;     	/* queue of tasks sleeping on LO_HI interrupt completion */
#endif
	int board_type;					/* an index for which board type for drivers that support multiple types. */
	int board_index;

	spinlock_t		smp_lock;        /* lock for critical section in ISR*/
	unsigned long	smp_flags;
        /***** Mutexes ****/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
        struct  mutex   ioctl_mtx;
#endif
	struct ai_board *next;     	    /* next board in linked list */
	struct	timeval		timestamp_read_start;	/* read timestamp start */
	struct	timeval		timestamp_read_end;		/* read timestamp end */
	struct	timeval		timestamp_btw_read_end;	/* timestamp b/w reads end */
	unsigned int		timestamp_read_hwm; /* timestamp read hwm */
	unsigned int		timestamp_btw_read_hwm; /* timestamp b/w read hwm */
	dev_region_t		mem_region[AI64_MAX_REGION];
	unsigned long		mmap_reg_select;   /* select Local GSC or PLX register*/
	unsigned int		*config_reg_address; /* configuration register
                                               space base address            */
	unsigned int		*local_reg_address;  /* Card registers base address  */
	unsigned short		num_dma_buffers;	/* number of dma buffers */
	unsigned short		use_resmem;			/* use resem */
	unsigned short		buf_inuse_hwm;		/* buffer in use hwm */
	unsigned short		hw_init_during_read; /* hardware init during read */
	unsigned short		dma_abort_request;  /* DMA abort request */
	unsigned short		read_to_issue_dma;  /* read to issue dma */
	unsigned short		wait_for_dma_to_complete;/* wait for dma to complete */
	unsigned short		read_in_progress;	/* read in progress */
	unsigned short		return_16bit_samples;	/* return 16 bit samples */
	unsigned short		bytes_per_sample;	/* 2 for short or 4 for int */
	unsigned short		enable_test_pattern;/* for debug only */
	unsigned int		ReadCount;			/* for debug only */
	unsigned int		WriteCount;			/* for debug only */
	unsigned int		Test_Pattern;		/* for debug only */
	unsigned short		validate_chan_0;	/* validate channel 0 */
	unsigned short		chan0_index;		/* channel 0 index for validate */
	unsigned int		dtime;				/* debug time */
	unsigned short		nchans;				/* number of channels */
    unsigned int        *phys_mem;          /* physical memory: physical address    */
    unsigned int        *virt_mem;          /* physical memory: virtual address     */
    unsigned int        phys_mem_size;      /* physical memory: memory size - bytes */
    unsigned int        phys_mem_size_freed;/* physical memory: freed size */
    int                 bus;                /* device: pci bus number */
    int                 slot;               /* device: slot number */
    int                 func;               /* device: function */
    uint                firmware;           /* device: firmware revision */
    uint                irq_added;          /* IRQ added flag */
    int                 ioctl_processing;   /* ioctl processing flag */
	double				MasterClockFreq;	/* Master Clock Frequency */
};

/* static function prototypes */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
long unlocked_device_ioctl(struct file *filp,u_int iocmd,unsigned long ioarg);
#endif
static int device_ioctl(struct inode *inode, struct file *fp, unsigned int num, unsigned long arg);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
static irqreturn_t device_interrupt(int irq, void *dev, struct pt_regs *regs);
#else
static irqreturn_t device_interrupt(int irq, void *dev_id);
#endif

/* timing and delay stuff */
#define INIT_TIMEOUT_MS       	2000 	    /* initialization timeout */
#define AUTOCAL_TIMEOUT_MS    	5000  	    /* auto calibration timeout */
#define CH_READY_TIMEOUT_MS   	500   	    /* timeout after channel setup operations */
#define MSECS_TO_SLEEP(MS)      ((MS * HZ) / 1000)

/* additional, internally used interrupt event codes */
#define NO_EVENT      	      	(-1)

#if (LARGE_FIFO == 1)
#define	MAX_DMA_SAMPLES		(256*1024)		/* 256 K samples */
#else
#define	MAX_DMA_SAMPLES		(64*1024)		/* 64 K samples */
#endif

#define	DMA_SAMPLES			(MAX_DMA_SAMPLES/2)	/* use half fifo */
#define	DMA_SIZE			(DMA_SAMPLES * sizeof(int))
#define	DMA_ORDER			(get_order(DMA_SIZE))

/* DMA command codes */
#define SETUP_DMA_CMD_0         0x09
#define STARTUP_DMA_CMD_0       0x03
#define START_DMA_CMD_1         0xB00
#define STOP_DMA_CMD_0_MASK     0x0C
#define STOP_DMA_CMD_1_MASK     (0x0C<<16)
#define NON_DEMAND_DMA_MODE     0x00020D43
#define DEMAND_DMA_MODE			(0x00020D43|1<<12)
#define DMA_ABORT_CMD_0			0x04
#define DMA_CMD_STAT_INT_CLEAR  0x08
#define PCI_INT_ENABLE        	0x00050900
#define LOCAL_CH0_DMA_INT_ENA   0x00040000

/* DMA Status codes */
#define NO_DMA_INPROGRESS          0
#define DMA_INPROGRESS             1
#define DMA_COMPLETE               2
#define DMA_TIMEDOUT               3
#define DMA_WAKEUP_PENDING         4

/* Macros to make register access easier */
/* Mapping Local Configuration Registers */
#define PciLocRange0(card)      ((card)->config_reg_address + PCI_TO_LOC_ADDR_0_RNG)
#define PciLocRemap0(card)      ((card)->config_reg_address + LOC_BASE_ADDR_REMAP_0)
#define ModeArb(card)           ((card)->config_reg_address + MODE_ARBITRATION)
#define EndianDescr(card)       ((card)->config_reg_address + BIG_LITTLE_ENDIAN_DESC)
#define PciLERomRange(card)     ((card)->config_reg_address + PCI_TO_LOC_ROM_RNG)
#define PciLERomRemap(card)     ((card)->config_reg_address + LOC_BASE_ADDR_REMAP_EXP_ROM)
#define PciLBRegDescr0(card)    ((card)->config_reg_address + BUS_REG_DESC_0_FOR_PCI_LOC)
#define LocPciRange(card)       ((card)->config_reg_address + DIR_MASTER_TO_PCI_RNG)
#define LocPciMemBase(card)     ((card)->config_reg_address + LOC_ADDR_FOR_DIR_MASTER_MEM)
#define LocPciIOBase(card)      ((card)->config_reg_address + LOC_ADDR_FOR_DIR_MASTER_IO)
#define LocPciRemap(card)       ((card)->config_reg_address + PCI_ADDR_REMAP_DIR_MASTER)
#define LocPciConfig(card)      ((card)->config_reg_address + PCI_CFG_ADDR_DIR_MASTER_IO)
#define PciLocRange1(card)      ((card)->config_reg_address + PCI_TO_LOC_ADDR_1_RNG)
#define PciLocRemap1(card)      ((card)->config_reg_address + LOC_BASE_ADDR_REMAP_1)
#define PciLBRegDescr1(card)    ((card)->config_reg_address + BUS_REG_DESC_1_FOR_PCI_LOC)

/* Mapping Runtime Registers */
#define Mailbox0(card)          ((card)->config_reg_address + MAILBOX_REGISTER_0)
#define Mailbox1(card)          ((card)->config_reg_address + MAILBOX_REGISTER_1)
#define Mailbox2(card)          ((card)->config_reg_address + MAILBOX_REGISTER_2)
#define Mailbox3(card)          ((card)->config_reg_address + MAILBOX_REGISTER_3)
#define Mailbox4(card)          ((card)->config_reg_address + MAILBOX_REGISTER_4)
#define Mailbox5(card)          ((card)->config_reg_address + MAILBOX_REGISTER_5)
#define Mailbox6(card)          ((card)->config_reg_address + MAILBOX_REGISTER_6)
#define Mailbox7(card)          ((card)->config_reg_address + MAILBOX_REGISTER_7)
#define PciLocDoorBell(card)    ((card)->config_reg_address + PCI_TO_LOC_DOORBELL)
#define LocPciDoorBell(card)    ((card)->config_reg_address + LOC_TO_PCI_DOORBELL)

/* Mapping interrupt control/status reg  */
#define IntCntrlStat(card)      ((card)->config_reg_address + AI64_INT_CTRL_STATUS)
#define RunTimeCntrl(card)      ((card)->config_reg_address + PROM_CTRL_CMD_CODES_CTRL)
#define DevVenIDc(card)         ((card)->config_reg_address + DEVICE_ID_VENDOR_ID)
#define RevID(card)             ((card)->config_reg_address + REVISION_ID)
#define MailboxReg0(card)       ((card)->config_reg_address + MAILBOX_REG_0)
#define MailboxReg1(card)       ((card)->config_reg_address + MAILBOX_REG_1)

/* Mapping DMA registers */
#define DMAMode0(card)          ((card)->config_reg_address + AI64_DMA_CH_0_MODE)
#define DMAPCIAddr0(card)       ((card)->config_reg_address + AI64_DMA_CH_0_PCI_ADDR)
#define DMALocalAddr0(card)     ((card)->config_reg_address + AI64_DMA_CH_0_LOCAL_ADDR)
#define DMAByteCnt0(card)       ((card)->config_reg_address + AI64_DMA_CH_0_TRANS_BYTE_CNT)
#define DMADescrPtr0(card)      ((card)->config_reg_address + AI64_DMA_CH_0_DESC_PTR)
#define DMAMode1(card)          ((card)->config_reg_address + AI64_DMA_CH_1_MODE)
#define DMAPCIAddr1(card)       ((card)->config_reg_address + AI64_DMA_CH_1_PCI_ADDR)
#define DMALocalAddr1(card)     ((card)->config_reg_address + AI64_DMA_CH_1_LOCAL_ADDR)
#define DMAByteCnt1(card)       ((card)->config_reg_address + AI64_DMA_CH_1_TRANS_BYTE_CNT)
#define DMADescrPtr1(card)      ((card)->config_reg_address + AI64_DMA_CH_1_DESC_PTR)
#define DMACmdStatus(card)      ((card)->config_reg_address + AI64_DMA_CMD_STATUS)
#define DMAArbitr(card)         ((card)->config_reg_address + AI64_DMA_MODE_ARB_REG)
#define DMAThreshold(card)      ((card)->config_reg_address + AI64_DMA_THRESHOLD_REG)

/* Mapping FIFO registers */
#define OutPostQIntStat(card)   ((card)->config_reg_address + OUT_POST_Q_INT_STATUS)
#define OutPostQIntMask(card)   ((card)->config_reg_address + OUT_POST_Q_INT_MASK)
#define MsgUnitCfg(card)        ((card)->config_reg_address + MSG_UNIT_CONFIG)
#define QBaseAddr(card)         ((card)->config_reg_address + Q_BASE_ADDR)
#define InFreeHeadPtr(card)     ((card)->config_reg_address + IN_FREE_HEAD_PTR)
#define InFreeTailPtr(card)     ((card)->config_reg_address + IN_FREE_TAIL_PTR)
#define InPostHeadPtr(card)     ((card)->config_reg_address + IN_POST_HEAD_PTR)
#define InPostTailPtr(card)     ((card)->config_reg_address + IN_POST_TAIL_PTR)
#define OutFreeHeadPtr(card)    ((card)->config_reg_address + OUT_FREE_HEAD_PTR)
#define OutFreeTailPtr(card)    ((card)->config_reg_address + OUT_FREE_TAIL_PTR)
#define OutPostHeadPtr(card)    ((card)->config_reg_address + OUT_POST_HEAD_PTR)
#define OutPostTailPtr(card)    ((card)->config_reg_address + OUT_POST_TAIL_PTR)
#define QStatusCtrl(card)       ((card)->config_reg_address + Q_STATUS_CTRL_REG)

/* Setup all the local registers */
#define BoardCtrlReg(card)      ((card)->local_reg_address + AI64_BOARD_CTRL_REG)
#define IntCtrlReg(card)		((card)->local_reg_address + AI64_INT_CTRL_REG)
#define InputBufferReg(card)    ((card)->local_reg_address + INPUT_DATA_BUFF_REG)
#define InputCtrlReg(card)      ((card)->local_reg_address + INPUT_BUFF_CTRL_REG)
#define RateAGenReg(card)		((card)->local_reg_address + AI64_RATE_A_GENERATOR_REG)
#define RateBGenReg(card)		((card)->local_reg_address + AI64_RATE_B_GENERATOR_REG)
#define BufferSizeReg(card)     ((card)->local_reg_address + BUFFER_SIZE_REG)
#define ScanSynchCtrlReg(card)  ((card)->local_reg_address + AI64_SCAN_SYNCH_CTRL_REG)

#define AI64_LOCK_INIT() { spin_lock_init(&ai64_device->smp_lock); }

/*** not sure why we need to lock other cards as well. Also, the lock
 *** appears to only lock the next board in line and not all the boards
 *** so that is also meaningless.
 ***/
#if 1
#define AI64_LOCK() {											\
	spin_lock_irqsave(&ai64_device->smp_lock, ai64_device->smp_flags);	\
}

#define AI64_UNLOCK() {													\
		spin_unlock_irqrestore(&ai64_device->smp_lock, ai64_device->smp_flags);	\
}
#else
#define AI64_LOCK() {											\
	spin_lock_irqsave(&ai64_device->smp_lock, ai64_device->smp_flags);	\
	if (ai64_device->next)											\
		spin_lock(&ai64_device->next->smp_lock);						\
}

#define AI64_UNLOCK() {													\
	if (ai64_device->next)													\
		spin_unlock(&ai64_device->next->smp_lock);							\
		spin_unlock_irqrestore(&ai64_device->smp_lock, ai64_device->smp_flags);	\
}
#endif


void writelocal(struct ai_board *,unsigned value, unsigned address);
unsigned readlocal(struct ai_board *,unsigned address);
void reg_dump(int dbglvl, struct ai_board *ai64_device, char *desc);
static int device_mmap(struct file *, struct vm_area_struct *);
