// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/driver/main.h $
// $Rev: 53732 $
// $Date: 2023-09-14 10:52:04 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Device Driver: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include "opto16x16.h"

#define	DEV_BAR_SHOW						0
#define	DEV_METRICS_SHOW					0
#define	DEV_PCI_ID_SHOW						0
#define	DEV_SUPPORTS_IRQ
#define	DEV_SUPPORTS_WAIT
#define	GSC_ALT_DEV_GET(a)					(a)
#define	GSC_ALT_STRUCT_GET(i,d)				(d)
#define	GSC_ALT_STRUCT_T					dev_data_t
#define	GSC_DEVS_PER_BOARD					1
typedef struct _dev_data_t					dev_data_t;

#include "gsc_main.h"



// macros *********************************************************************

#define	DEV_MODEL							"OPTO16X16"	// Upper case form of the below.
#define	DEV_NAME							"opto16x16"	// MUST AGREE WITH OPTO16X16_BASE_NAME
#define	DEV_NAME_LC							DEV_NAME
#define	DEV_NAME_UC							DEV_MODEL

#define	DEV_VERSION_STR						GSC_STR(DEV_VERSION_MAJ_NUM) "." GSC_STR(DEV_VERSION_MIN_NUM)
#define	DEV_VERSION							DEV_VERSION_STR

#define	DEV_VERSION_MAJ_NUM					2
#define	DEV_VERSION_MIN_NUM					6
// 2.6	Updated to support the 6.x series kernel.
// 2.5	LINTed sources.
//		Updated to support segregation of PLX specific sources.
//		Updated comments for consistency.
// 2.4	Fixed code in which open succeeds even if initialization fails.
//		Updated for porting to Windows.
//		Removed DMA code as it is unused.
// 2.3	Added support for the 5.x kernel series.
//		Added Endianness support.
// 2.2	Bug fix: was ignoring BAR setup return value.
//		Updated per changes to the OS specific PCI services.
//		Made updates for DEV_SUPPORTS_XXX macro changes.
//		Modified code so open succeeds even if initialization fails.
//		Defined macros for the WAIT services.
// 2.1	The BAR code has been updated to include the data under a single structure.
//		The register definitions have been updated.
//		I/O services now pass around an os_mem_t structure instead of a buffer pointer.
//		White space cleanup.
//		Changed the arbitrary wait event callback argument to a void* type.
//		Modified to allow multiple apps to simultaneously access a single device.
//		Correct access to BAR0 and BAR1 (is RO, was RW).
//		Corrected the IOCTL code for the register write and mod services.
// 2.0	Updated to use the newer common driver sources.
//		Removed GNU notice from non-Linux specific files.
//		Removed Linux specific content from non-Linux specific source files.
//		White space cleanup.
//		Now using a spinlock rather than enabling and disabling interrupts.
//		Updated gsc_irq_open() and gsc_irq_close().
//		Updated gsc_dma_open() and gsc_dma_close().
// 1.5	Reduced #include list in driver interface header.
// 1.4	Updated for the 3.x kernel.
// 1.3	BAR0 and BAR2 are now the only BAR regions used.
//		Include all common source, though not all are used.
// 1.2	Updated the set of IOCTL services.
//		Changed use of DEV_SUPPORTS_READ macro.
//		Changed use of DEV_SUPPORTS_WRITE macro.
//		Changed use of DEV_IO_AUTO_START macro.
//		Eliminated the global dev_check_id() routine.
// 1.1	Updated the susbsystem id searched for.
// 1.0-BETA	Initial release.

// WAIT services
#define	DEV_WAIT_GSC_ALL					OPTO16X16_WAIT_GSC_ALL
#define	DEV_WAIT_ALT_ALL					OPTO16X16_WAIT_ALT_ALL
#define	DEV_WAIT_IO_ALL						OPTO16X16_WAIT_IO_ALL



// data types *****************************************************************

// This is not used, but is required.
struct _dev_io_t
{
	// Initialized by open and IOCTL services (initialize and/or service specific).

	// none

	// Initialized by I/O service at time of use.

	gsc_dma_ch_t*		dma_channel;		// Use this channel for DMA.
	u32					io_reg_offset;		// Offset of device's I/O FIFO.
};

struct _dev_data_t
{
	os_pci_t			pci;			// The kernel PCI device descriptor.
	os_data_t			os;				// OS specific data.
	os_spinlock_t		spinlock;		// Control ISR access.
	os_sem_t			sem;			// Control thread access.
	gsc_dev_type_t		board_type;		// Corresponds to basic board type.
	const char*			model;			// Base model number in upper case.
	int					board_index;	// Index of the board being accessed.
	int					users;			// Number of currently active open() requests.
	int					share;			// Were we opened in shared mode?

	gsc_bar_t			bar;			// device register mappings
	gsc_dma_t			dma;			// For DMA based I/O.
	gsc_irq_t			irq;			// For interrupts.
	gsc_wait_node_t*	wait_list;

	struct
	{
		// This is here to meet ISR requirements.
		VADDR_T			plx_intcsr_32;		// Interrupt Control/Status Register
		VADDR_T			plx_dmaarb_32;		// DMA Arbitration Register
		VADDR_T			plx_dmathr_32;		// DMA Threshold Register

		VADDR_T			gsc_bcsr_8;			// Board Control/Status Register
		VADDR_T			gsc_cdr_32;			// Clock Division Register
		VADDR_T			gsc_cier_16;		// COS Interrupt Enable Register
		VADDR_T			gsc_cosr_16;		// Change of State Register
		VADDR_T			gsc_cpr_16;			// COS Polarity Register
		VADDR_T			gsc_odr_16;			// Output Data Register
		VADDR_T			gsc_rdr_16;			// Receive Data Register
		VADDR_T			gsc_recr_16;		// Receive Event Counter Register
	} vaddr;
};



// prototypes *****************************************************************

int		dev_irq_create(dev_data_t* dev);
void	dev_irq_destroy(dev_data_t* dev);

int		initialize_ioctl(dev_data_t* dev, void* arg);



#endif
