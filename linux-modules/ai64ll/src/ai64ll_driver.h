/******************************************************************************
 *                                                                            *
 * File:         ai64ll_driver.h                                              *
 *                                                                            *
 * Description:  Driver specific global definitions.                          *
 *                                                                            *
 * Date:         02/15/2018                                                   *
 * History:                                                                   *
 *                                                                            *
 *   7  2/15/18 P. Ongini                                                     *
 *      Updated company information                                           *
 *                                                                            *
 *   6  12/15/2014 D. Dubash                                                  * 
 *      The driver only uses interrupts for board Initialization complete     *
 *      and AutoCal complete. The driver has been made more robust so as      *
 *      as to poll in case the interrupt fails to operate properly on         *
 *      some motherboards.                                                    *
 *                                                                            *
 *   5  1/15/09 D. Dubash                                                     *
 *      Support for RH 5.2.x                                                  *
 *                                                                            *
 *   4  9/02/08 D. Dubash                                                     *
 *      Support for RH 5.1.x                                                  *
 *                                                                            *
 *   3  3/07/07 D. Dubash                                                     *
 *      Support for 64 bit architecture.                                      *
 *                                                                            *
 *   2  1/11/07 D. Dubash                                                     *
 *      Support for new ioctl IOCTL_AI64LL_MMAP_GET_OFFSET which replaces     *
 *      ioctl IOCTL_MMAP_SELECT                                               *
 *                                                                            *
 *   1  7/10/03 D. Dubash                                                     *
 *      This is a driver header file.                                         *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *  Copyright (C) 2003 and beyond Concurrent Real-Time, Inc.                  *
 *  All rights reserved                                                       *
 *                                                                            *
 ******************************************************************************/

#ifndef __AI64LL_DRIVER__
#define __AI64LL_DRIVER__

#define AI64LL_MMAP_SUPPORT        /* drd */

#if 0 /* use void* instead of uintptr_t */
/* if 64-bit architecture */
#if (defined(__i386__) && defined(CONFIG_HIGHMEM) && defined(CONFIG_HIGHMEM64G)) || defined(__x86_64__) || defined (__ia64__) || defined(__mips64__) || (defined(__mips__) && defined(CONFIG_HIGHMEM) && defined(CONFIG_64BIT_PHYS_ADDR))
typedef u64 uintptr_t;
#else /* 32-bit architecture */
typedef u32 uintptr_t;
#endif
#endif

/*** drd ***/
#define AI64LL_DMA_MEM_ALLOC(size) __get_dma_pages(GFP_KERNEL,get_order(size))
#define AI64LL_DMA_MEM_FREE(addr, size) free_pages(addr, get_order(size))

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#define LINUX_2_4
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
#define LINUX_2_2
#else
#define LINUX_2_0
#endif

#define N0_BLOCK		  0
#define DMA_READ		  0
#define DMA_WRITE		  1

#define MAX_BOARDS        10 /* maximum number of boards that driver supports. Arbitrarily chosen */

#define INITIALIZE_TIMEOUT_MS    2000
#define AUTO_CAL_TIMEOUT_MS 	 10000

#define MSECS_TO_SLEEP(MS)       ((MS * HZ)/1000)
#define SECS_TO_SLEEP(S)         (S * HZ)

enum {
	CONFIG_REGION,		/* Configuration Register - Region 0 */
	IO_REGION,			/* IO Register            - Region 1 */
	LOCAL_REGION,		/* Local Register         - Region 2 */

	/****/
	AI64LL_MAX_REGION	/* Must be last entry in enum */
};

/*  */
/*
*	The below structure is used to record information about regions of
*	I/O or memory address space that are occupied by the device.
*/

typedef struct
{
	u32	address;
	u32	size;
	u32 flags;
} dev_region_t;

/*
 * Description:
 *		Driver-defined structure used to hold
 *		miscellaneous device information.
 */
struct ai64ll_device
{
      struct pci_dev 	 *pdev;		/* structure for the pcidevice info */
      unsigned int	     device_id; 	/* Device id 			    */
      unsigned int	     vendor_id; 	/* Vendor id 			    */
      unsigned int       subsys_id;	/* Subsystem id of the card found   */
	  dev_region_t		 mem_region[AI64LL_MAX_REGION];
      unsigned int       major;         /* Major number for the driver      */
      unsigned int  	 minor; 	/* Minor number for the device      */
      unsigned int       irq;		/* Irq no. allocated for the device */
      unsigned int 	     busy;		/* Device state 	 	    */

      volatile unsigned int	int_rcvd;	/* interrupt recvd flag - drd */

#ifdef LINUX_2_4
      wait_queue_head_t ioctlwq;
#else
      struct wait_queue *ioctlwq;	/* Wait queue for PIO ioctl operation */
#endif

      int               timeout_value;	/* Timeout value		    */
      int               timeout_retry_count; /* Timeout retry count */

      unsigned int   	error;		/* Recent error type occured	    */
      unsigned int      debug_buffer[AI64LL_DEBUG_SIZE]; /* Debug messages buffer  */

      unsigned int  	initialize;	 /* Initialize status 		    */
      unsigned int   	autocalibration; /* Autocailbration status 	    */
      /*** Modification: Following 6 fields added: 21-12-01  *********/

      ai64ll_int_info_t int_notify;	 /* Notify app of these interrupts. */
      ai64ll_int_info_t int_status;	 /* Interrupt status for IOCTL */

      spinlock_t        smp_lock;        /* lock for critical section in ISR*/
	  unsigned long     smp_flags;

      u32	            *config_reg_address; /* configuration register
                                               space base address            */
      u32               *local_reg_address;  /* Card registers base address  */
      u32               chan_regs[AI64LL_MAX_CHANS];
#ifdef AI64LL_MMAP_SUPPORT	/* drd */
      unsigned long     mmap_reg_select;   /* select Local GSC or PLX register*/
#endif /* END AI64LL_MMAP_SUPPORT */
      struct ai64ll_device *next;        /* linked list pointer          */
      int               bus;             /* device: pci bus number */
      int               slot;            /* device: slot number */
      int               func;            /* device: function */
      uint              firmware;        /* device: firmware revision */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
      struct mutex      ioctl_mtx;
#endif
      int               ioctl_processing;           /* ioctl processing flag */
};

static struct fasync_struct *ai64ll_fasyncptr;      /* pointer to fasync structure  */

/*
 * Prototypes for globally defined functions...
 */

#ifdef AI64LL_MMAP_SUPPORT
static int 
ai64ll_mmap(struct file *, struct vm_area_struct *);
#endif /* AI64LL_MMAP_SUPPORT */

int
ai64ll_open (struct inode *, struct file *);

int
ai64ll_release (struct inode *,struct file *);

ssize_t
ai64ll_read (struct file *,char *,size_t, loff_t *);

ssize_t
ai64ll_write (struct file *,const char *,size_t, loff_t *);

int
ai64ll_ioctl (struct inode *,struct file *, unsigned int, unsigned long);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
long unlocked_ai64ll_ioctl(struct file *filp,u_int iocmd,unsigned long ioarg );
#endif

static int
fasync_ai64ll ( int, struct file*, int);

static irqreturn_t
ai64ll_irq_handler ( int, void *);

int
read_from_buffer (struct ai64ll_device *, unsigned int *, int, int);

int
write_to_buffer (struct ai64ll_device *, int, int);

void
write_using_dma (struct ai64ll_device *, int, int);

struct ai64ll_pci_config_regs
read_pci_config_regs (struct ai64ll_device *);

struct ai64ll_local_config_regs
read_local_config_regs (struct ai64ll_device *);

#define PCI_INT_ENABLE	0x00050900

/*****************************************************************************/
/*** AI64LL Registers ***/
#define BoardCtrlReg                (ai64ll_device->local_reg_address + \
                                        AI64LL_REGISTER_INDEX(AI64LL_GSC_BCR))
#define IntCtrlReg                  (ai64ll_device->local_reg_address + \
                                        AI64LL_REGISTER_INDEX(AI64LL_GSC_ICR))
#define ScanSyncCtrlReg             (ai64ll_device->local_reg_address + \
                                        AI64LL_REGISTER_INDEX(AI64LL_GSC_SSCR))
#define InputBufferControl          (ai64ll_device->local_reg_address + \
                                        AI64LL_REGISTER_INDEX((AI64LL_GSC_IBCR))
#define FirmwareRevision            (ai64ll_device->local_reg_address + \
                                        AI64LL_REGISTER_INDEX(AI64LL_GSC_FREV))
#define ConvCounter                 (ai64ll_device->local_reg_address + \
                                        AI64LL_REGISTER_INDEX(AI64LL_GSC_CCNT))

/*** DMA Registers ***/
#define DmaCh0Mode 		     (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMAMODE0))
#define DmaCh0PciAddr        (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMAPADR0))
#define DmaCh0LocalAddr      (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMALADR0))
#define DmaCh0TransByteCnt   (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMASIZ0))
#define DmaCh0DescPtr        (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMADPR0))
#define DmaCh1Mode           (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMAMODE1))
#define DmaCh1PciAddr        (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMAPADR1))
#define DmaCh1LocalAddr      (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMALADR1))
#define DmaCh1TransByteCnt   (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMASIZ1))
#define DmaCh1DescPtr        (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMADPR1))
#define DmaCmdStatus         (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMACSR01))
#define DmaModeArbReg        (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMAARB))
#define DmaThresholdReg      (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMATHR ))

/*** PCI Configuration Registers ***/
#define DeviceVendorId           (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_IDR))
#define StatusCommand            (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_CR_SR))
#define ClassCodeRevision_Id     (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_REV_CCR))
#define BistHdrTypeLatCacheSize  (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_CLSR_LTR_HTR_BISTR))

/*************** Start of Modification : 21-12-01 *********************/

#define PciMemBaseAddr          (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_BAR0))
#define PciIoBaseAddr           (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_BAR1))
#define PciBaseAddr0            (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_BAR2))
#define PciBaseAddr_1           (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_BAR3))

/*************** End of Modification : 21-12-01 ***********************/

#define CardbusCisPtr           (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_CIS))
#define SubsysIdVendorId        (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_SVID_SID))
#define PciBaseAddrLocRom       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_ERBAR))
#define LatGntIntPinLine        (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PCI_ILR_IPR_MGR_MLR ))

/*** Local Configuration Registers. ***/
#define PciToLocAddr0Rng     (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_LASORR ))
#define LocBaseAddrRemap0      (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_LAS0BA))
#define ModeArbitration        (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MARBR))
#define BigLittleEndianDesc    (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_BIGEND))
#define PciToLocRomRng         (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_EROMRR))
#define LocBaseAddrRemapExpRom (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_EROMBA))
#define BusRegDesc0ForPciLoc   (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_LBRD0))
#define DirMasterToPciRng      (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMRR))
#define LocAddrForDirMasterMem (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMLBAM))
#define LocAddrForDirMasterIo  (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMLBAI))
#define PciAddrRemapDirMaster  (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMPBAM))
#define PciCfgAddrDirMasterIo  (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_DMCFGA))
#define PciToLocAddr1Rng       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_LAS1RR))
#define LocBaseAddrRemap1      (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_LAS1BA))
#define BusRegDesc1ForPciLoc   (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_LBRD1))

/*** Run Time Registers ***/
#define MailboxRegister0       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MBOX0))
#define MailboxRegister1       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MBOX1))
#define MailboxRegister2       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MBOX2))
#define MailboxRegister3       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MBOX3))
#define MailboxRegister4       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MBOX4))
#define MailboxRegister5       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MBOX5))
#define MailboxRegister6       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MBOX6))
#define MailboxRegister7       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MBOX7))
#define PciToLocDoorbell       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_P2LDBELL))
#define LocToPciDoorbell       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_L2PDBELL))
#define IntCtrlStatus          (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_INTCSR))
#define PromCtrlCmdCodesCtrl   (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_CNTRL))
#define DeviceIdVendorId       (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_PCIHIDR))
#define RevisionId             (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_PCIHREV))
#define MailboxReg0            (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MBOX0_ALT))
#define MailboxReg1            (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MBOX1_ALT ))

/*** Messaging Queue Registers ***/
#define InQPort                (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_IQP))
#define OutQPort               (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_OQP))
#define MsgUnitConfig          (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_MQCR))
#define QBaseAddr              (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_QBAR))
#define InFreeHeadPtr          (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_IFHPR))
#define InFreeTailPtr          (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_IFTPR))
#define InPostHeadPtr          (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_IPHPR))
#define InPostTailPtr          (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_IPTPR))
#define OutFreeHeadPtr         (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_OFHPR))
#define OutFreeTailPtr         (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_OFTPR))
#define OutPostHeadPtr         (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_OPHPR))
#define OutPostTailPtr         (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_OPTPR))
#define QStatusCtrlReg         (ai64ll_device->config_reg_address + \
                                    AI64LL_REGISTER_INDEX(AI64LL_PLX_QSR))
/*****************************************************************************/

#if 0
/*** AI64LL Registers ***/
#define BOARD_CTRL_REG              (ai64ll_device->local_reg_address + AI64LL_GSC_BCR)
#define INT_CTRL_REG                (ai64ll_device->local_reg_address + AI64LL_GSC_ICR)
#define SCAN_SYNC_CTRL_REG          (ai64ll_device->local_reg_address + AI64LL_GSC_SSCR)
#define INPUT_BUFFER_CONTROL        (ai64ll_device->local_reg_address + AI64LL_GSC_IBCR)
#define FIRMWARE_REVISION           (ai64ll_device->local_reg_address + AI64LL_GSC_FREV)
#define CONV_COUNTER                (ai64ll_device->local_reg_address + AI64LL_GSC_CCNT)

/*** DMA Registers ***/
#define DMA_CH_0_MODE 		     (ai64ll_device->config_reg_address + AI64LL_PLX_DMAMODE0)
#define DMA_CH_0_PCI_ADDR            (ai64ll_device->config_reg_address + AI64LL_PLX_DMAPADR0)
#define DMA_CH_0_LOCAL_ADDR          (ai64ll_device->config_reg_address + AI64LL_PLX_DMALADR0)
#define DMA_CH_0_TRANS_BYTE_CNT      (ai64ll_device->config_reg_address + AI64LL_PLX_DMASIZ0)
#define DMA_CH_0_DESC_PTR            (ai64ll_device->config_reg_address + AI64LL_PLX_DMADPR0)
#define DMA_CH_1_MODE                (ai64ll_device->config_reg_address + AI64LL_PLX_DMAMODE1)
#define DMA_CH_1_PCI_ADDR            (ai64ll_device->config_reg_address + AI64LL_PLX_DMAPADR1)
#define DMA_CH_1_LOCAL_ADDR          (ai64ll_device->config_reg_address + AI64LL_PLX_DMALADR1)
#define DMA_CH_1_TRANS_BYTE_CNT      (ai64ll_device->config_reg_address + AI64LL_PLX_DMASIZ1)
#define DMA_CH_1_DESC_PTR            (ai64ll_device->config_reg_address + AI64LL_PLX_DMADPR1)
#define DMA_CMD_STATUS               (ai64ll_device->config_reg_address + AI64LL_PLX_DMACSR01)
#define DMA_MODE_ARB_REG             (ai64ll_device->config_reg_address + AI64LL_PLX_DMAARB)
#define DMA_THRESHOLD_REG            (ai64ll_device->config_reg_address + AI64LL_PLX_DMATHR )

/*** PCI Configuration Registers ***/
#define DEVICE_VENDOR_ID              (ai64ll_device->config_reg_address + AI64LL_PCI_IDR)
#define STATUS_COMMAND                (ai64ll_device->config_reg_address + AI64LL_PCI_CR_SR)
#define CLASS_CODE_REVISION_ID        (ai64ll_device->config_reg_address + AI64LL_PCI_REV_CCR)
#define BIST_HDR_TYPE_LAT_CACHE_SIZE  (ai64ll_device->config_reg_address + AI64LL_PCI_CLSR_LTR_HTR_BISTR)

/*************** Start of Modification : 21-12-01 *********************/

#define PCI_MEM_BASE_ADDR             (ai64ll_device->config_reg_address + AI64LL_PCI_BAR0)
#define PCI_IO_BASE_ADDR              (ai64ll_device->config_reg_address + AI64LL_PCI_BAR1 )
#define PCI_BASE_ADDR_0               (ai64ll_device->config_reg_address + AI64LL_PCI_BAR2)
#define PCI_BASE_ADDR_1               (ai64ll_device->config_reg_address + AI64LL_PCI_BAR3)

/*************** End of Modification : 21-12-01 ***********************/

#define CARDBUS_CIS_PTR               (ai64ll_device->config_reg_address + AI64LL_PCI_CIS)
#define SUBSYS_ID_VENDOR_ID           (ai64ll_device->config_reg_address + AI64LL_PCI_SVID_SID)
#define PCI_BASE_ADDR_LOC_ROM         (ai64ll_device->config_reg_address + AI64LL_PCI_ERBAR)
#define LAT_GNT_INT_PIN_LINE          (ai64ll_device->config_reg_address + AI64LL_PCI_ILR_IPR_MGR_MLR )

/*** Local Configuration Registers. ***/
#define PCI_TO_LOC_ADDR_0_RNG         (ai64ll_device->config_reg_address + AI64LL_PLX_LASORR )
#define LOC_BASE_ADDR_REMAP_0         (ai64ll_device->config_reg_address + AI64LL_PLX_LAS0BA)
#define MODE_ARBITRATION              (ai64ll_device->config_reg_address + AI64LL_PLX_MARBR)
#define BIG_LITTLE_ENDIAN_DESC        (ai64ll_device->config_reg_address + AI64LL_PLX_BIGEND)
#define PCI_TO_LOC_ROM_RNG            (ai64ll_device->config_reg_address + AI64LL_PLX_EROMRR)
#define LOC_BASE_ADDR_REMAP_EXP_ROM   (ai64ll_device->config_reg_address + AI64LL_PLX_EROMBA)
#define BUS_REG_DESC_0_FOR_PCI_LOC    (ai64ll_device->config_reg_address + AI64LL_PLX_LBRD0)
#define DIR_MASTER_TO_PCI_RNG         (ai64ll_device->config_reg_address + AI64LL_PLX_DMRR)
#define LOC_ADDR_FOR_DIR_MASTER_MEM   (ai64ll_device->config_reg_address + AI64LL_PLX_DMLBAM)
#define LOC_ADDR_FOR_DIR_MASTER_IO    (ai64ll_device->config_reg_address + AI64LL_PLX_DMLBAI)
#define PCI_ADDR_REMAP_DIR_MASTER     (ai64ll_device->config_reg_address + AI64LL_PLX_DMPBAM)
#define PCI_CFG_ADDR_DIR_MASTER_IO    (ai64ll_device->config_reg_address + AI64LL_PLX_DMCFGA)
#define PCI_TO_LOC_ADDR_1_RNG         (ai64ll_device->config_reg_address + AI64LL_PLX_LAS1RR)
#define LOC_BASE_ADDR_REMAP_1         (ai64ll_device->config_reg_address + AI64LL_PLX_LAS1BA)
#define BUS_REG_DESC_1_FOR_PCI_LOC    (ai64ll_device->config_reg_address + AI64LL_PLX_LBRD1)

/*** Run Time Registers ***/
#define MAILBOX_REGISTER_0           (ai64ll_device->config_reg_address + AI64LL_PLX_MBOX0)
#define MAILBOX_REGISTER_1           (ai64ll_device->config_reg_address + AI64LL_PLX_MBOX1)
#define MAILBOX_REGISTER_2           (ai64ll_device->config_reg_address + AI64LL_PLX_MBOX2)
#define MAILBOX_REGISTER_3           (ai64ll_device->config_reg_address + AI64LL_PLX_MBOX3)
#define MAILBOX_REGISTER_4           (ai64ll_device->config_reg_address + AI64LL_PLX_MBOX4)
#define MAILBOX_REGISTER_5           (ai64ll_device->config_reg_address + AI64LL_PLX_MBOX5)
#define MAILBOX_REGISTER_6           (ai64ll_device->config_reg_address + AI64LL_PLX_MBOX6)
#define MAILBOX_REGISTER_7           (ai64ll_device->config_reg_address + AI64LL_PLX_MBOX7)
#define PCI_TO_LOC_DOORBELL          (ai64ll_device->config_reg_address + AI64LL_PLX_P2LDBELL)
#define LOC_TO_PCI_DOORBELL          (ai64ll_device->config_reg_address + AI64LL_PLX_L2PDBELL)
#define INT_CTRL_STATUS              (ai64ll_device->config_reg_address + AI64LL_PLX_INTCSR)
#define PROM_CTRL_CMD_CODES_CTRL     (ai64ll_device->config_reg_address + AI64LL_PLX_CNTRL)
#define DEVICE_ID_VENDOR_ID          (ai64ll_device->config_reg_address + AI64LL_PLX_PCIHIDR)
#define REVISION_ID                  (ai64ll_device->config_reg_address + AI64LL_PLX_PCIHREV)
#define MAILBOX_REG_0                (ai64ll_device->config_reg_address + AI64LL_PLX_MBOX0_ALT)
#define MAILBOX_REG_1                (ai64ll_device->config_reg_address + AI64LL_PLX_MBOX1_ALT )

/*** Messaging Queue Registers ***/
#define IN_Q_PORT                    (ai64ll_device->config_reg_address + AI64LL_PLX_IQP)
#define OUT_Q_PORT                   (ai64ll_device->config_reg_address + AI64LL_PLX_OQP)
#define MSG_UNIT_CONFIG              (ai64ll_device->config_reg_address + AI64LL_PLX_MQCR)
#define Q_BASE_ADDR                  (ai64ll_device->config_reg_address + AI64LL_PLX_QBAR)
#define IN_FREE_HEAD_PTR             (ai64ll_device->config_reg_address + AI64LL_PLX_IFHPR)
#define IN_FREE_TAIL_PTR             (ai64ll_device->config_reg_address + AI64LL_PLX_IFTPR)
#define IN_POST_HEAD_PTR             (ai64ll_device->config_reg_address + AI64LL_PLX_IPHPR)
#define IN_POST_TAIL_PTR             (ai64ll_device->config_reg_address + AI64LL_PLX_IPTPR)
#define OUT_FREE_HEAD_PTR            (ai64ll_device->config_reg_address + AI64LL_PLX_OFHPR)
#define OUT_FREE_TAIL_PTR            (ai64ll_device->config_reg_address + AI64LL_PLX_OFTPR)
#define OUT_POST_HEAD_PTR            (ai64ll_device->config_reg_address + AI64LL_PLX_OPHPR)
#define OUT_POST_TAIL_PTR            (ai64ll_device->config_reg_address + AI64LL_PLX_OPTPR)
#define Q_STATUS_CTRL_REG            (ai64ll_device->config_reg_address + AI64LL_PLX_QSR )
#endif

#ifdef	__SMP__	
#define AI64LL_LOCK_INIT() { spin_lock_init(&ai64ll_device->smp_lock); }

#define AI64LL_LOCK() {														\
  spin_lock_irqsave(&ai64ll_device->smp_lock, ai64ll_device->smp_flags);		\
  if (ai64ll_device->next)													\
      spin_lock(&ai64ll_device->next->smp_lock);								\
}

#define AI64LL_UNLOCK() {													\
  if (ai64ll_device->next)													\
      spin_unlock(&ai64ll_device->next->smp_lock);							\
  spin_unlock_irqrestore(&ai64ll_device->smp_lock, ai64ll_device->smp_flags);	\
}

#else	/* ELSE __SMP__ */

#define AI64LL_LOCK_INIT() {}
#define AI64LL_LOCK() { disable_irq(ai64ll_device->irq); }
#define AI64LL_UNLOCK() { enable_irq(ai64ll_device->irq); }

#endif	/* END __SMP__ */


#endif // __AI64LL_DRIVER__
