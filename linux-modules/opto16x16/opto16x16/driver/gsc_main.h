// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_main.h $
// $Rev: 52733 $
// $Date: 2023-04-03 14:56:38 -0500 (Mon, 03 Apr 2023) $

// OS & Device Independent: Device Driver: header file

#ifndef	__GSC_MAIN_H__
#define	__GSC_MAIN_H__

#define	DEV_QTY_MAX					32

#include "os_main.h"
#include "gsc_main_pci.h"



// macros *********************************************************************

// This is the OVERALL version number. This includes the different PCI/PCIe implementations.
#define	GSC_DRIVER_VERSION			DEV_VERSION "." GSC_COMMON_VERSION "." OS_COMMON_VERSION

// This is for the common code only!
#define	GSC_COMMON_VERSION_NUM		104
// 104	Added RX DEBUG capabilities.
// 103	Updated comments for consistency.
// 102	Added comments to gsc_common.h regarding the 16AO16FLV.
// 101	Added support for the 24DSI6LN4AO.
// 100	Removed unintended debug output statements.
// 99	Renamed gsc_irq_reset_pci to gsc_irq_cpi_reset for consistency.
//		Segregated additional PLX specific code.
//		Updated system message to begin with the driver module name.
//		Began implementation for MSI support.
// 98	Segregated all PLX specific code to accommodate alternate device implementations.
// 97	Updated debugging display output.
// 96	LINTed sources.
// 95	Added temporary experimental write code in gsc_write.c for performance testing.
//		The code is partially retained but is not included for compilation.
// 94	Fixed a DMA bug relating to the use of a timeout setting of zero.
// 93	Corrected endian error message.
//		Added Endianess support for the PEX8111 and PEX8112.
// 92	Fixed a timeout bug in gsc_write_pio_work_32_bit().
// 91	Updated IRQ code for improved clarity and modularity.
//		Updated DMA code to support multiple DMA implementations.
//		Removed I/O debug code.
// 90	Removed compiler warnings under Windows.
// 89	Fixed a IOCTL bug in which alt->sem wasn't being unlocked.
//		Increased modularization for porting effort to another OS.
//		Added support for the ADADIO2.
//		Removed compiler warnings under Windows.
// 88	Made DMA code more modular.
// 87	Fixed 9060 Endian support bug.
// 86	Added support for Big Endian hosts.
//		Updated per OS specific DMA memory allocation interface.
// 85	Fixed bug in gsc_dma_perform(): timeouts were being ignored.
//		Fixed bug in gsc_dma_perform(): the timeout detection logic was incorrect.
// 84	Added support for the 18AISS8AO8.
//		Improved register validation.
// 83	Subsystem Vendor ID and Subsystem ID can each be ignored if -1 in device table.
//		#define for GSC_REG_ENCODE_MASK is moved here from gsc_reg.c.
//		Enhanced error reporting for the process of validating register definitions.
// 82	Fixed bug in DMA completion code.
//		Updated initialization for the write service.
// 81	Updated muti-channel ISR code for new I/O streams structure.
// 80	Added support for the 16AISS2AO2A2M.
//		DEV_WAIT_IO_ALL is now required if WAIT is supported along with READ or WRITE
//		I/O WAIT macros are now defined by the device API and are passed to GSC code in dev_io_t.
//		Updated Block Mode DMA macro name (_DMA to _BMDMA).
//		Added I/O, DMA and register access debug code.
//		Added stream selection field in the "size" argument for the read and writecalls.
// 79	Added OS IRQ calls: os_irq_create() and os_irq_destroy()
// 78	Renamed field in gsc_irq_t for clarity.
// 77	Fixed a bug in which wait list node removal didn't always have the list locked.
//		Added support for some DMA metrics.
//		Improved IOCTL argument validation.
// 76	Moved a typedef to device specific code.
// 75	Udated some utility services: applying a setting, end by reading it back
// 74	Updated for support of some drivers under older kernels.
//		Added support for the 16AI32SSC1M.
// 73	Updated #include files for consistancy.
// 72	Fixed a DMA timeout calculation bug.
// 71	Updated to accommodate porting to Windows KMDF.
// 70	Updated per changes to the low level register interface.
//		Updated the driver initialization logic.
//		Updated IOCTL init and reset code.
//		Added support to exclude the WAIT feature.
//		Updated macros to include/exclude IRQ support.
//		Fixed bug in init code where last PCI device is one of ours.
// 69	Fixed register definitions for the PEX8111 and PEX8112.
// 68	Added special case for register I/O where register id value is 0.
//		Added support for inifinite I/O timeout.
//		Improved response to being signalled (i.e. being told to abort).
// 67	Modified to allow multiple apps to simultaneously access a single device.
//		An event wait timeout value of zero now means do not timeout.
//		Fixed an I/O bug.
//		Updated the version number macros.
// 66	Updated the format and encoding of register macros.
// 65	Fixed DMA bug.
// 64	Moved some BAR functionality to the OS specific code.
//		I/O now passes around an os_mem_t structure instead of a buffer pointer.
//		Copying to and from user space is now done with more use specific routines.
//		Made more ISR functionality subject to the GSC_IRQ_ISR_FLAG_DETECT_ONLY flag.
//		Dropped I/O register access services.
//		Performed some LINTing.
// 63	Simplified error messages.
//		Fixed ul2hex().
// 62	The macro S32_MAX is defined only if not already done so.
// 61	Added the utility service ul2hex().
//		Changed register offsets to u16 from unsigned long.
//		Changed printk's to printf's.
//		Removed an RTX compiler warning in gsc_poll_u32.
//		Expanded id data for the 24DSI32 options.
//		Made ISR access more OS agnostic.
//		Made PCI registers accessible from inside an ISR.
// 60	Added gsc_wait_resume_irq_io() for use by the ISR.
// 59	Updated sources for use by all drivers.
//		Updated the VPD structure's head field to be an index rather than a pointer.
//		Updated gsc_irq_open() and gsc_irq_close().
//		Updated gsc_dma_open() and gsc_dma_close().
//		Fixed a bug in the DMA usage tracking code.
//		Added DIO32 support.
//		Updated ABORT interrupt implementation..
//		Added persistent debug debug statements; commented out though.
//		Added Local Bus Parity Error interrupt handling.
// 58	Migrated from using gsc_common/linux/driver/*.[ch] to using
//		gsc_common/driver/*.[ch] and gsc_common/driver/linux/*.[ch].
//		The migration involved primarily .c and .h sources, but also a few others.
//		All previos version data for the older gsc_main.h remains with that file.
//		Version numbers 47 through 57 in this file have been skipped for
//		version number consistency.
// 47	Added support for the DIO40.
// 46	Added support for the 16AO4MF.
//		Added support for the 24DSI12WRCIEPE.
//		Added support for the 24DSI6C500K.
// 45	Removed dead code (i.e. excluded by #if 0).
// 44	More cross-OS porting.
//		All interrupts are processed before awaiting threads are resumed.
// 43	Added support for the 16AO64C, the 16AI64 and the DIO24.
//		Added support for calling external code for a DMA reset.
// 42	Removed warning from strict compilation.
// 41	Code cleanup.
//		Merge of limited content from the Linux only source tree.
// 40	Made mods to support the INtime Windows real-time extension.
//		Reorganized source tree and enhanced code modularity.
//		This code supports devices with multiple data streams (i.e. SIO4).
//		This is a modified version of the single stream driver tree.
// 39	Added support for boards that need additional setup after a DMA channel
//		has been selected.
// 38	Added support for the 18AO8.
//		Made the IOCTL buffer larger - 512 bytes.
// 37	Updated for the 3.x kernel.
// 36	Additional SIO4 porting.
//		Added support (via notes only) for the PCIE104-24DSI12.
// 35	Removed compile warnings from the copy_from_user_ret macro.
// 34	All common driver sources are included even though not all are used.
// 33	Modified the kernel_2_X.h sources for use by the SIO4 driver.
//		The SIO4 support porting is only partially complete at this time.
//		Implemented GSC_DEVS_PER_BOARD macro for multi-device boards.
//		Added support for the PLX PEX 8112 PCI Express Bridge.
// 32	Updated information that distinguishes 16HSDI from 16SDI-HS.
// 31	Fixed bugs in VPD code (Vital Product Data).
// 30	Added support for the PLX PEX 8111 PCI Express Bridge.
//		Added GSC_IRQ_NOT_USED macro to hide interrupt servicing.
//		Added GSC_PCI_SPACE_SIZE macro to support the PEX8111.
// 29	Fixed a bug in the GSC_INTCSR_MAILBOX_INT_ACTIVE macro.
//		Removed a compiler warning under Fedora 15.
// 28	Added support for the 16AI64SSC.
//		Added macros for D0 through D31.
//		Modified gsc_sem_t to prevent hanging on an uninitialized structure.
//		Moved dev_check_id() functionality to dev_device_create().
//		Added IOCTL support for the 16AI64SSA/C Low Latency Read service.
// 27	Added support for the 20AOF16C500KR.
// 26	Added support for the 16AICS32.
// 25	Added support for deprication of file_ops.ioctl.
//		Added support for the 18AISS6C boards.
//		Started PLX EEPROM access support.
//		Modified so BAR2 can now be memory or I/O mapped.
// 24	Added support for the 25DSI20C500K boards.
// 23	Added support for the 16AI64SSA boards.
//		Corrected the _1M macro.
// 22	Added support for the OPTO32 boards.
//		Added support for the PCI9060ES.
// 21	Removed compiler warning in Fedora 12: reduced module_init stack usage.
// 20	Added support for the 16AIO168.
//		Removed remove_proc_entry() call from proc_start - fix for Fedora 14.
//		Fixed a bug in gsc_ioctl_init().
// 19	Added support for the 24DSI16WRC.
// 18	Added common PIO read and write routines.
//		Changed use of DEV_SUPPORTS_PROC_ID_STR macro.
//		Changed use of DEV_SUPPORTS_READ macro.
//		Changed use of DEV_SUPPORTS_WRITE macro.
//		Added initial support for Vital Product Data.
//		Added support for the 16AI32SSA.
// 17	Added support for the OPTO16X16.
//		Corrected a bug: wait timeouts in jiffy units are negative.
// 16	Added support for aborting active I/O operations.
//		Added support for Auto-Start operations.
//		Added wait options for I/O cancellations.
//		Fixed bugs in the DMA code evaluating lock return status.
//		Added /proc board identifier string support.
//		Added gsc_irq_local_disable and gsc_irq_local_enable;
//		Added failure message when driver doesn't load.
//		Improved timeout handling.
//		Added gsc_time.c.
// 15	Added support for the 14HSAI4 and the HPDI32.
// 14	Added wait event, wait cancel and wait status services.
// 13	Modified the EVENT_WAIT_IRQ_TO() macro for the 2.6 kernel.
//		We now no longer initialize the condition variable in the macro.
//		The condition variable must be initialized outside the macro.
//		Added support for the 16AIO and the 12AIO.
// 12	Added more id information for the 18AI32SSC1M boards.
// 11	Added module count support.
//		Added common gsc_open() and gsc_close() calls.
// 10	Added support for the 12AISS8AO4 and the 16AO16.
// 9	Added support for the 16HSDI4AO4. Fixed bug in gsc_ioctl_init().
// 8	Added support for the 16AISS16AO2.
//		Made various read() and write() support services use same data types.
// 7	Added support for the 16AO20. Implemented write() support.
// 6	Added dev_check_id() for more detailed device identification.
// 5	Fixed DMA engine initialization code.
//		This was previously and incorrectly reported here as a version 4 mod.
// 4	Added support for the 18AI32SSC1M.
//		Modified some PLX register names for consistency.
// 3	Added support for the 16AI32SSC.
// 2	Added support for the 24DSI6.
// 1	Updated the makefile's "clean" code.
//		Added code to expand access rights to makefile.dep.
//		Added 24DSI12/32 types to gsc_common.h
// 0	initial release
#define	GSC_COMMON_VERSION			GSC_COMMON_VERSION_STR
#define	GSC_COMMON_VERSION_STR		GSC_STR(GSC_COMMON_VERSION_NUM)

#define	__GSC_STR(x)				# x
#define	GSC_STR(x)					__GSC_STR(x)
#define	SIZEOF_ARRAY(a)				(sizeof((a))/sizeof((a)[0]))

#ifndef	ECANCELED
#define	ECANCELED					131
#endif

#define	_1K							(1024L)
#define	_5K							(_1K * 5)
#define	_30K						(_1K * 30)
#define	_32K						(_1K * 32)
#define	_64K						(_1K * 64)
#define	_128K						(_1K * 128L)
#define	_220K						(_1K * 220L)
#define	_256K						(_1K * 256L)
#define	_512K						(_1K * 512L)
#define	_1100K						(_1K * 1100L)
#define	_1M							(_1K * _1K)
#define	_2M							(_1M * 2L)
#define	_4M							(_1M * 4L)
#define	_8M							(_1M * 8L)

#define	_1MHZ						( 1000000L)
#define	_8MHZ						( 8000000L)
#define	_9_6MHZ						( 9600000L)
#define	_16MHZ						(16000000L)
#define	_19_2MHZ					(19200000L)
#define	_20MHZ						(20000000L)
#define	_30MHZ						(30000000L)
#define	_38_4MHZ					(38400000L)

// Bit definitions
#define	D0							0x00000001
#define	D1							0x00000002
#define	D2							0x00000004
#define	D3							0x00000008
#define	D4							0x00000010
#define	D5							0x00000020
#define	D6							0x00000040
#define	D7							0x00000080
#define	D8							0x00000100
#define	D9							0x00000200
#define	D10							0x00000400
#define	D11							0x00000800
#define	D12							0x00001000
#define	D13							0x00002000
#define	D14							0x00004000
#define	D15							0x00008000
#define	D16							0x00010000
#define	D17							0x00020000
#define	D18							0x00040000
#define	D19							0x00080000
#define	D20							0x00100000
#define	D21							0x00200000
#define	D22							0x00400000
#define	D23							0x00800000
#define	D24							0x01000000
#define	D25							0x02000000
#define	D26							0x04000000
#define	D27							0x08000000
#define	D28							0x10000000
#define	D29							0x20000000
#define	D30							0x40000000
#define	D31							0x80000000

#ifndef	TRUE
	#define	TRUE							1
#endif

#ifndef	FALSE
	#define	FALSE							0
#endif

// Flags for gsc_irq_isr_common()
#define	GSC_IRQ_ISR_FLAG_LOCK				0x1		// Lock upon entry, unlock on exit
#define	GSC_IRQ_ISR_FLAG_DETECT_ONLY		0x2		// Detect the interrupt only, don't take care of it.

// Virtual address items
#define	GSC_VADDR(d,o)						(VADDR_T) (((u8*) (d)->bar.bar[2].vaddr) + (o))

// DMA
#define	GSC_DMA_CAP_BMDMA_READ				0x01	// DMA chan can do BMDMA Rx
#define	GSC_DMA_CAP_BMDMA_WRITE				0x02	// DMA chan can do BMDMA Tx
#define	GSC_DMA_CAP_DMDMA_READ				0x04	// DMA chan can do DMDMA Rx
#define	GSC_DMA_CAP_DMDMA_WRITE				0x08	// DMA chan can do DMDMA Tx
#define	GSC_DMA_CAP_DEV_MIN					0x10	// Device specific capabilities start here

#define	GSC_DMA_SEL_STATIC					0x10	// Get the DMA chan and keep it.
#define	GSC_DMA_SEL_DYNAMIC					0x20	// Hold the DMA chan only as needed.

#define	GSC_REG_TYPE_ACCESS_RO				0	// read-only
#define	GSC_REG_TYPE_ACCESS_RW				1	// read/write

#ifndef	GSC_REG_TYPE_PCI_ACCESS
#define	GSC_REG_TYPE_PCI_ACCESS				GSC_REG_TYPE_ACCESS_RO
#endif

// Data size limits.
#ifndef	S32_MAX
#define	S32_MAX								(+2147483647L)
#endif

#ifndef	GSC_DEVS_PER_BOARD
#error	ERROR: UNDEFINED MACRO: GSC_DEVS_PER_BOARD
// #define	GSC_DEVS_PER_BOARD				1
// #define	GSC_DEVS_PER_BOARD				4
#endif

#if (GSC_DEVS_PER_BOARD < 1)
#error	ERROR: INVALID MACRO DEFINITION: GSC_DEVS_PER_BOARD
// #define	GSC_DEVS_PER_BOARD				1
// #define	GSC_DEVS_PER_BOARD				4
#endif

#ifndef	GSC_ALT_STRUCT_T
#error	ERROR: UNDEFINED MACRO: GSC_ALT_STRUCT_T
// #define	GSC_ALT_STRUCT_T					dev_data_t
// #define	GSC_ALT_STRUCT_T					chan_data_t
#endif

#ifndef	GSC_ALT_DEV_GET
#error	ERROR: UNDEFINED MACRO: GSC_ALT_DEV_GET
// #define	GSC_ALT_DEV_GET(a)				(a)
// #define	GSC_ALT_DEV_GET(a)				(a)->dev
#endif

#ifndef	GSC_ALT_STRUCT_GET
#error	ERROR: UNDEFINED MACRO: GSC_ALT_STRUCT_GET
// #define	GSC_ALT_STRUCT_GET(i,d)			(d)
// #define	GSC_ALT_STRUCT_GET(i,d)			(d)->channel[i]
#endif

#ifndef	ETIMEDOUT
	#define	ETIMEDOUT						70		// Operation timed out
#endif

#define	GSC_REG_ENCODE_MASK_DEFAULT			GSC_REG_ENCODE((u32) ~0,4,(u32) ~0)

#ifndef	GSC_REG_ENCODE_MASK
	// A device's main.h defines this if it uses any of the normally unused bits.
	#define	GSC_REG_ENCODE_MASK				GSC_REG_ENCODE_MASK_DEFAULT
#endif

#ifdef DEV_SUPPORTS_IRQ
#if (GSC_DEVS_PER_BOARD > 1)
	#define	GSC_WAIT_RESUME_IRQ_MAIN(dev,flags)			gsc_wait_resume_irq_main_multi(dev,flags)
	#define	GSC_WAIT_RESUME_IRQ_MAIN_DMA(dev,dma,flags)	gsc_wait_resume_irq_main_dma_multi(dev,dma,flags)
#else
	#define	GSC_WAIT_RESUME_IRQ_MAIN(dev,flags)			gsc_wait_resume_irq_main(dev,flags)
	#define	GSC_WAIT_RESUME_IRQ_MAIN_DMA(dev,dma,flags)	gsc_wait_resume_irq_main(dev,flags)
#endif
#endif

#if defined(DEV_SUPPORTS_READ)

	#if defined(RX_DEBUG)
		#define	RX_DEBUG_ADD(test,var,add)	if ((test) >= 0) (var) += (add)
	#else
		#define	RX_DEBUG_ADD(test,var,add)
	#endif

#endif



// data types *****************************************************************

typedef struct _dev_io_t	dev_io_t;	// A device specific structure.

typedef	int					(*gsc_ioctl_service_t)(GSC_ALT_STRUCT_T* alt, void* arg);

typedef struct
{
	u8						qty;
	os_bar_t				bar[6];			// device register mappings
} gsc_bar_t;

typedef struct
{
	// only one can be required, both can be optional
	int						mem;	// 0 = optional, >0 = required, <0 = do not map
	int						io;		// 0 = optional, >0 = required, <0 = do not map
	int						rw;		// 0 = read-only, !0 = read/write
} gsc_bar_map_t;

typedef struct
{
	gsc_bar_map_t			bar[6];
} gsc_bar_maps_t;

typedef struct
{
	const char*				model;	// NULL model terminates a list of entries.
	u16						vendor;
	u16						device;
	s32						sub_vendor;	// if -1, then it is ignored.
	s32						sub_device;	// if -1, then it is ignored.
	gsc_dev_type_t			type;
} gsc_dev_id_t;

typedef struct
{
	os_sem_t				sem;		// control access
	gsc_dma_ch_t			channel[2];	// Settings and such
	u32						usage_map;
	struct
	{
		int					count;		// Number of waiting threads.
		os_sem_t			sem;		// waiting for a free channel
	} wait;
} gsc_dma_t;

typedef struct
{
	os_sem_t				sem;		// Control access.
	os_irq_t				os;
	int						opened;
	u32						usage_map;
	u32						did;		// For various feature tests.
	u32						isr_mask;	// The ISR looks only at these bits.
} gsc_irq_t;

typedef struct
{
	int						cmd;	// -1 AND
	gsc_ioctl_service_t		func;	// NULL terminate the list
} gsc_ioctl_t;

typedef struct
{
	int						initialized;	// Has this code been initialized?
	int						available;		// Is VPD available on this device?
	int						pci_reg_offset;	// Offset of the PCI VPD registers.
	int						loaded;			// Have we loaded the image?
	int						image_offset;	// index of first VPD byte
	u8						image[1025];	// VPD image, Not a multiple of 4 bytes!
} gsc_vpd_data_t;

typedef struct _gsc_wait_node_t
{
	gsc_wait_t*					wait;
	os_event_t					evnt;
	os_time_t					tt_start;
	struct _gsc_wait_node_t*	next;
} gsc_wait_node_t;



// variables ******************************************************************

extern	gsc_global_t		gsc_global;



// prototypes *****************************************************************

int		gsc_bar_create(dev_data_t* dev, gsc_bar_t* bar, const gsc_bar_maps_t* map);
void	gsc_bar_destroy(gsc_bar_t* bar);

int		gsc_close(GSC_ALT_STRUCT_T* alt);

int				gsc_dma_abort(dev_data_t* dev, dev_io_t* io);
int				gsc_dma_abort_pci(dev_data_t* dev, gsc_dma_ch_t* dma);
gsc_dma_ch_t*	gsc_dma_acquire_pci(dev_data_t* dev, u32 ability);
void			gsc_dma_close(dev_data_t* dev, int index);
int				gsc_dma_close_pci(dev_data_t* dev);
int				gsc_dma_create(dev_data_t* dev, u32 ch0_flags, u32 ch1_flags);
int				gsc_dma_create_pci(dev_data_t* dev, u32 ch0_flg, u32 ch1_flg);
void			gsc_dma_destroy(dev_data_t* dev);
int				gsc_dma_finish_pci(gsc_dma_setup_t* setup);
int				gsc_dma_open(dev_data_t* dev, int index);
int				gsc_dma_open_pci(dev_data_t* dev);
long			gsc_dma_perform(gsc_dma_setup_t* setup);
int				gsc_dma_release_pci(dev_data_t* dev, gsc_dma_ch_t* dma);
int				gsc_dma_setup_pci(gsc_dma_setup_t* setup);
int				gsc_dma_start_pci(GSC_ALT_STRUCT_T* alt, void* arg);

int		gsc_eeprom_access(dev_data_t* dev);
int		gsc_eeprom_access_pci(dev_data_t* dev);
int		gsc_endian_init(dev_data_t* dev);
int		gsc_endian_init_pci(dev_data_t* dev);

int		gsc_init_dev_create(dev_data_t* dev);
int		gsc_init_dev_data_t_alloc(dev_data_t** dev);
void	gsc_init_dev_data_t_free(dev_data_t** dev);
void	gsc_init_dev_destroy(dev_data_t* dev);
int		gsc_io_create(GSC_ALT_STRUCT_T* alt, dev_io_t* gsc, size_t size);
void	gsc_io_destroy(GSC_ALT_STRUCT_T* alt, dev_io_t* gsc);
int		gsc_ioctl(GSC_ALT_STRUCT_T* alt, unsigned int cmd, void* arg);
int		gsc_ioctl_init(void);
void	gsc_ioctl_reset(void);
void	gsc_irq_access_lock(dev_data_t* dev);
void	gsc_irq_access_unlock(dev_data_t* dev);
void	gsc_irq_close(dev_data_t* dev, int index);
int		gsc_irq_create(dev_data_t* dev);
int		gsc_irq_create_pci(dev_data_t* dev);
void	gsc_irq_destroy(dev_data_t* dev);
void	gsc_irq_destroy_pci(dev_data_t* dev);
int		gsc_irq_init_pci(dev_data_t* dev, int lock);
int		gsc_irq_isr_common(void* dev_id, u32 flags);
int		gsc_irq_isr_common_pci(dev_data_t* dev, u32 flags);
int		gsc_irq_local_disable(dev_data_t* dev);
int		gsc_irq_local_disable_pci(dev_data_t* dev);
int		gsc_irq_local_enable(dev_data_t* dev);
int		gsc_irq_local_enable_pci(dev_data_t* dev);
int		gsc_irq_open(dev_data_t* dev, int index);
int		gsc_irq_reset_pci(dev_data_t* dev, int lock);

int		gsc_macro_test_base_name(const char* name);
int		gsc_macro_test_model(void);

int		gsc_open(GSC_ALT_STRUCT_T* alt, int share);

int		gsc_poll_u32(	dev_data_t*	dev,
						size_t		ms_limit,
						VADDR_T		vaddr,
						u32			mask,
						u32			value);

long	gsc_read(GSC_ALT_STRUCT_T* alt, dev_io_t* io, void* buf, size_t count);
int		gsc_read_abort_active_xfer(GSC_ALT_STRUCT_T* alt, dev_io_t* io);

// Must define GSC_READ_PIO_WORK and/or GSC_READ_PIO_WORK_XX_BIT
long	gsc_read_pio_work			(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);
long	gsc_read_pio_work_8_bit		(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);
long	gsc_read_pio_work_16_bit	(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);
long	gsc_read_pio_work_32_bit	(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);

void	gsc_reg_mod(GSC_ALT_STRUCT_T* alt, u32 reg, u32 val, u32 mask);	// GSC REGISTERS ONLY!
int		gsc_reg_mod_ioctl(GSC_ALT_STRUCT_T* alt, gsc_reg_t* arg);
u32		gsc_reg_read(GSC_ALT_STRUCT_T* alt, u32 reg);					// GSC REGISTERS ONLY!
int		gsc_reg_read_ioctl(GSC_ALT_STRUCT_T* alt, gsc_reg_t* arg);
void	gsc_reg_write(GSC_ALT_STRUCT_T* alt, u32 reg, u32 val);			// GSC REGISTERS ONLY!
int		gsc_reg_write_ioctl(GSC_ALT_STRUCT_T* alt, gsc_reg_t* arg);

int		gsc_s32_list_reg(dev_data_t* dev, s32* value, const s32* list, VADDR_T vaddr, int begin, int end);
int		gsc_s32_list_var(s32* value, const s32* list, s32* var);
int		gsc_s32_range_reg(dev_data_t* dev, s32* value, s32 min, s32 max, VADDR_T vaddr, int begin, int end);
int		gsc_s32_range_var(s32* value, s32 min, s32 max, s32* var);

int		gsc_vpd_read_ioctl(dev_data_t* dev, gsc_vpd_t* vpd);

int		gsc_wait_cancel_ioctl(GSC_ALT_STRUCT_T* alt, gsc_wait_t* arg);
void	gsc_wait_close(GSC_ALT_STRUCT_T* alt);
int		gsc_wait_event(	GSC_ALT_STRUCT_T*	alt,
						gsc_wait_t*			wait,
						int					(*setup)(GSC_ALT_STRUCT_T* alt, void* arg),
						void*				arg,
						os_sem_t*			sem);
int		gsc_wait_event_ioctl(GSC_ALT_STRUCT_T* alt, gsc_wait_t* arg);
void	gsc_wait_resume_io(GSC_ALT_STRUCT_T* alt, u32 io);
void	gsc_wait_resume_irq_alt(GSC_ALT_STRUCT_T* alt_t, u32 alt);
void	gsc_wait_resume_irq_gsc(GSC_ALT_STRUCT_T* alt, u32 gsc);
void	gsc_wait_resume_irq_io(GSC_ALT_STRUCT_T* alt, u32 io);
void	gsc_wait_resume_irq_main(GSC_ALT_STRUCT_T* alt, u32 main);
void	gsc_wait_resume_irq_main_dma_multi(dev_data_t* dev, gsc_dma_ch_t* dma, u32 flags);
void	gsc_wait_resume_irq_main_multi(dev_data_t* dev, u32 flags);
int		gsc_wait_status_ioctl(GSC_ALT_STRUCT_T* alt, gsc_wait_t* arg);
long	gsc_write(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const void* buf, size_t count);
int		gsc_write_abort_active_xfer(GSC_ALT_STRUCT_T* alt, dev_io_t* io);

// Must define GSC_WRITE_PIO_WORK and/or GSC_WRITE_PIO_WORK_XX_BIT
long	gsc_write_pio_work			(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);
long	gsc_write_pio_work_8_bit	(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);
long	gsc_write_pio_work_16_bit	(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);
long	gsc_write_pio_work_32_bit	(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const os_mem_t* mem, size_t count, os_time_tick_t st_end);

int		ul2hex(unsigned long ul, char* dest);



// ****************************************************************************
// THESE ARE PROVIDED BY THE DEVICE SPECIFIC CODE

//#define	DEV_MODEL					"XXX"
//#define	DEV_NAME					"xxx"
//#define	DEV_VERSION					"x.x"
//#define	DEV_BAR_SHOW				0 or 1 (1 to show BAR info during init)
//#define	DEV_PCI_ID_SHOW				0 or 1 (1 to show ID info during init)
//#define	DEV_SUPPORTS_READ			define if read() is supported.
//#define	DEV_SUPPORTS_WRITE			define if write() is supported.
//#define	DEV_SUPPORTS_PROC_ID_STR	define if this string is supported.
//#define	DEV_SUPPORTS_VPD			Is Vital Product Data supported?

// Variables
extern	const gsc_dev_id_t	dev_id_list[];
extern	const gsc_ioctl_t	dev_ioctl_list[];

// Functions
int		dev_close					(GSC_ALT_STRUCT_T* alt);
int		dev_device_create			(dev_data_t* dev);
void	dev_device_destroy			(dev_data_t* dev);
void	dev_irq_isr_local_handler	(dev_data_t* dev);
int		dev_open					(GSC_ALT_STRUCT_T* alt);
int		dev_reg_mod_alt				(GSC_ALT_STRUCT_T* alt, gsc_reg_t* arg);
int		dev_reg_read_alt			(GSC_ALT_STRUCT_T* alt, gsc_reg_t* arg);
int		dev_reg_write_alt			(GSC_ALT_STRUCT_T* alt, gsc_reg_t* arg);



#endif
