// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/main.h $
// $Rev: 54971 $
// $Date: 2024-08-07 15:59:52 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Device Driver: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include "16ai64ssa.h"

#define	DEV_BAR_SHOW						0
#define	DEV_METRICS_SHOW					0
#define	DEV_PCI_ID_SHOW						0
#define	DEV_SUPPORTS_IRQ
#define	DEV_SUPPORTS_READ
#define	DEV_SUPPORTS_WAIT
#define	GSC_ALT_DEV_GET(a)					(a)
#define	GSC_ALT_STRUCT_GET(i,d)				(d)
#define	GSC_ALT_STRUCT_T					dev_data_t
#define	GSC_DEVS_PER_BOARD					1

typedef struct _dev_data_t					dev_data_t;

#include "gsc_main.h"



// macros *********************************************************************

#define	DEV_MODEL							"16AI64SSA"	// Upper case form of the below.
#define	DEV_NAME							"16ai64ssa"	// MUST AGREE WITH AI64SSA_BASE_NAME


#define	DEV_VERSION							"3.13"		// FOR DEVICE SPECIFIC CODE ONLY!
// 3.13	Updated for Fedora 38.
//		Updated build scripts for consistency.
// 3.12	Updated to support the 6.x series kernel.
//		Renamed all Auto_Cal content to Autocal.
//		Renamed all Auto_Cal_Sts content to Autocal_Status.
//		Updated comments for consistency.
//		Updated the Initialize IOCTL service for consistency.
//		Updated the Autocal IOCTL service for consistency.
//		Updated the Input Buffer Clear IOCTL service for consistency.
// 3.11	Standardized various IOCTL system log messages.
//		Standardized processing of the initialization IOCTL service.
//		Standardized processing of the autocalibration IOCTL service.
// 3.10	Updated initial file content for consistency.
//		Added Endianness support.
//		Updated for porting to Windows.
//		Added support for the 5.x kernel series.
//		Deleted useless code in ioctl.c.
//		Updated some comments for consistency.
// 3.9	Corrected comment in main.h.
// 3.8	Updated per changes to common code.
//		Modified code so open succeeds even if initialization fails.
// 3.7	Bug fix: was ignoring BAR setup return value.
//		Updated per changes to the OS specific PCI services.
//		Made updates for DEV_SUPPORTS_XXX macro changes.
//		Added some internal documentation.
// 3.6	Added an infinite I/O timeout option.
//		Added conditional reporting of device metrics during driver startup.
//		Corrected the IOCTL code for the register write and mod services.
// 3.5	Correct access to BAR0 and BAR1 (is RO, was RW).
//		Corrected FIFO size computation.
// 3.4	Improved ISR's handling of interrupts, especially for firmware with the irq anomaly.
//		Corrected spelling of "anomaly".
//		Added query options AI64SSA_QUERY_CHAN_RANGE, AI64SSA_QUERY_REG_ACAR and AI64SSA_QUERY_IRQ1_BUF_ERROR.
// 3.3	The BAR code has been updated to include the data under a single structure.
//		The register definitions have been updated.
//		I/O services now pass around an os_mem_t structure instead of a buffer pointer.
//		White space cleanup.
//		Changed the arbitrary wait event callback argument to a void* type.// 3.2	White space cleanup.
//		Modified to allow multiple apps to simultaneously access a single device.
// 3.2	?????????????????????????????
// 3.1	I/O code now passes an os_mem_t structure rather than a buffer pointer.
//		Thread arguments are not void* data types.
// 3.0	Updated to use the newer common driver sources.
//		Removed GNU notice from non-Linux specific files.
//		Removed Linux specific content from non-Linux specific source files.
//		White space cleanup.
//		Now using a spinlock rather than enabling and disabling interrupts.
//		Updated gsc_irq_open() and gsc_irq_close().
//		Updated gsc_dma_open() and gsc_dma_close().
//		Enhanced output reported when the DEV_PCI_ID_SHOW option is enabled.
//		Updated the I/O buffer size.
// 2.10	Modified Low Latency Read requests for 32-channel devices.
//		Added support for the Low Latency Control Register.
// 2.9	Reduced set of #includes in the driver interface header file.
// 2.8	Corrected the definitions of the Voltage Range macros.
// 2.7	Corrected a system time tick count roll over bug in ioctl.c and device.c.
// 2.6	Removed extraneous debug messages.
// 2.5	Updated for the 3.x kernel.
//		Modified ISR and ICR usage to work with ICR anomaly of older firmware.
// 2.4	Include all common source, though not all are used.
//		Split the utility code into two libraries: common and device specific.
// 2.3	Corrected a bug that prevented DMDMA from being used.
// 2.2	Removed compiler warnings for Fedora 15.
// 2.1	Removed some debug code in irq.c.
// 2.0	Overhauled driver.
// 1.4	Removed some compile errors and warnings.
// 1.3	Updated various Autocalibration related strings and macros for consistency.
// 1.2	Updated to the 2.6.27 kernel.
// 1.1	Fixed a DMA read bug for low data rates. Updated GSC_NAME macro.

// I/O services
#define	DEV_IO_STREAM_QTY					(DEV_IO_RX_STREAM_QTY + DEV_IO_TX_STREAM_QTY)
#define	DEV_IO_RX_STREAM_QTY				1
#define	DEV_IO_TX_STREAM_QTY				0

#define	GSC_READ_PIO_WORK_32_BIT

// WAIT services
#define	DEV_WAIT_GSC_ALL					AI64SSA_WAIT_GSC_ALL
#define	DEV_WAIT_ALT_ALL					AI64SSA_WAIT_ALT_ALL
#define	DEV_WAIT_IO_ALL						AI64SSA_WAIT_IO_ALL



// data types *****************************************************************

struct _dev_io_t
{
	// Initialized by open and IOCTL services (initialize and/or service specific).

	s32					io_mode;			// PIO, DMA, DMDMA
	s32					overflow_check;		// Check overflow when reading?
	s32					pio_threshold;		// Use PIO if samples <= this.
	s32					timeout_s;			// I/O timeout in seconds.
	s32					underflow_check;	// Check underflow when reading?

	// Initialized by I/O service at time of use.

	int					abort;
	gsc_dma_ch_t*		dma_channel;		// Use this channel for DMA.
	int					non_blocking;		// Is this non-blocking I/O?

	// Initialized by device specific I/O create code.

	int					bytes_per_sample;	// Sample size in bytes.
	u32					io_reg_offset;		// Offset of device's I/O FIFO.
	VADDR_T				io_reg_vaddr;		// Address of device's I/O FIFO.

	void				(*dev_io_sw_init)		(GSC_ALT_STRUCT_T* alt, dev_io_t* io);
	void				(*dev_io_close)			(GSC_ALT_STRUCT_T* alt, dev_io_t* io);
	void				(*dev_io_open)			(GSC_ALT_STRUCT_T* alt, dev_io_t* io);
	int					(*dev_io_startup)		(GSC_ALT_STRUCT_T* alt, dev_io_t* io);
	long				(*dev_pio_available)	(GSC_ALT_STRUCT_T* alt, dev_io_t* io, size_t count);
	long				(*dev_bmdma_available)	(GSC_ALT_STRUCT_T* alt, dev_io_t* io, size_t count);
	long				(*dev_dmdma_available)	(GSC_ALT_STRUCT_T* alt, dev_io_t* io, size_t count);
	long				(*dev_pio_xfer)			(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);
	long				(*dev_bmdma_xfer)		(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);
	long				(*dev_dmdma_xfer)		(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);

	struct
	{
		u32				abort;		// ...WAIT_IO_XXX_ABORT
		u32				done;		// ...WAIT_IO_XXX_DONE
		u32				error;		// ...WAIT_IO_XXX_ERROR
		u32				timeout;	// ...WAIT_IO_XXX_TIMEOUT
	} wait;

	// Initialize by GSC commone create code.

	os_sem_t			sem;				// Only one Tx or Rx at a time.
	os_mem_t			mem;				// I/O buffer.
};

struct _dev_data_t
{
	os_pci_t			pci;			// The kernel PCI device descriptor.
	os_data_t			os;				// OS specific data.
	os_spinlock_t		spinlock;		// Control ISR access.
	os_sem_t			sem;			// Control thread access.
	gsc_dev_type_t		board_type;		// Corresponds to basic device type.
	const char*			model;			// Base model number in upper case.
	int					board_index;	// Index of the device being accessed.
	int					users;			// Number of currently active open() requests.
	int					share;			// Were we opened in shared mode?

	gsc_bar_t			bar;			// device register mappings
	gsc_dma_t			dma;			// For DMA based I/O.
	gsc_irq_t			irq;			// For interrut support.
	gsc_wait_node_t*	wait_list;

	struct
	{					// This is for streaming I/O
		dev_io_t		rx;			// Analog Input read
		dev_io_t*		io_streams[DEV_IO_STREAM_QTY];
	} io;

	struct
	{
		VADDR_T			plx_intcsr_32;	// Interrupt Control/Status Register
		VADDR_T			plx_dmaarb_32;	// DMA Arbitration Register
		VADDR_T			plx_dmathr_32;	// DMA Threshold Register

		VADDR_T			gsc_bctlr_32;	// 0x00 Board Control Register
		VADDR_T			gsc_icr_32;		// 0x04 Interrupt Control Register
		VADDR_T			gsc_ibcr_32;	// 0x0C Input Buffer Control Register
		VADDR_T			gsc_idbr_32;	// 0x08 Input Data Buffer Register

		VADDR_T			gsc_ragr_32;	// 0x10 Rate-A Generator Register
		VADDR_T			gsc_rbgr_32;	// 0x14 Rate-B Generator Register
		VADDR_T			gsc_bufsr_32;	// 0x18 Buffer Size Register
		VADDR_T			gsc_bursr_32;	// 0x1C Burst Size Register

		VADDR_T			gsc_sscr_32;	// 0x20 Scan & Sync Control Register
		VADDR_T			gsc_acar_32;	// 0x24 Active Channel Assignment Register
		VADDR_T			gsc_bcfgr_32;	// 0x28 Board Configuration Register

		VADDR_T			gsc_asiocr_32;	// 0x34 Auxiliary Sync I/O Control Register
		VADDR_T			gsc_smlwr_32;	// 0x38 Scan Marker Lower Word Register
		VADDR_T			gsc_smuwr_32;	// 0x3C Scan Marker Upper Word Register

		VADDR_T			gsc_llcr_32;	// 0x40 Low Latency Control Register

		VADDR_T			gsc_llhr00_16;	// 0x100 Low Latency Holding Register 00

	} vaddr;

	struct
	{
		u32				gsc_bcfgr_32;		// Board Configuration Register

		s32				autocal_ms;		// Maximum ms for autocalibration

		s32				channel_qty;		// The number of channels on the device.
		s32				channel_range;		// Is the channel range option available?
		s32				channels_max;		// Maximum channels supported by model.

		s32				data_packing;		// Is Data Packing supported in FW?

		u32				fifo_size;			// Size of FIFO - not the fill level.
		s32				fsamp_max;			// The maximum Fsamp rate per channel.
		s32				fsamp_min;			// The minimum Fsamp rate per channel.

		s32				icr_anomaly;		// Is the ICR anomaly present?
		s32				initialize_ms;		// Maximum ms for initialize
		s32				irq1_buf_error;		// Is this IRQ supported?

		s32				low_latency;		// Is Low Latency supported?

		s32				master_clock;		// Master clock frequency

		s32				nrate_max;			// Minimum Nrate value.
		s32				nrate_min;			// Maximum Nrate value.

		int				pci9080;			// Is this a PCI9080 model device?

		s32				rate_gen_qty;		// The number of rate generators on the device.

		s32				reg_acar;			// Active Channel Assignment Reg. supported?
		s32				reg_llcr;			// Low Latency Control Register supported?

	} cache;
};



// prototypes *****************************************************************

void		dev_io_close(dev_data_t* dev);
int			dev_io_create(dev_data_t* dev);
void		dev_io_destroy(dev_data_t* dev);
int			dev_io_open(dev_data_t* dev);
dev_io_t*	dev_io_read_select(dev_data_t* dev, size_t count);
int			dev_irq_create(dev_data_t* dev);
void		dev_irq_destroy(dev_data_t* dev);
int			dev_read_create(dev_data_t* dev, dev_io_t* io);

int			initialize_ioctl(dev_data_t* dev, void* arg);



#endif
