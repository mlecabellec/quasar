// vim:ts=4 expandtab: 
/******************************************************************************
 *                                                                            *
 * File:         ai64.c                                                       *
 *                                                                            *
 * Description:  Linux Host driver for PMC-16AI64SS card. This driver is      *
 *               an installable module.                                       *
 *                                                                            *
 * Revision:     6.3                                                          *
 * Date:         01/10/2014                                                   *
 * History:                                                                   *
 *                                                                            *
 *  28  01/10/2014 D. Dubash                                                  *
 *      - if the number of samples was too small, the driver was faulting     *
 *        when a add_timer was still pending and the user attemped to perform *
 *        a device initialization or auto cal. The fix is to make sure        *
 *        an active add_timer is deleted if a timeout has not occurred.       *
 *      - previously, DEMAND DMA was behaving same and regular DMA, i.e.      *
 *        driver would block until sufficient samples are available. Now,     *
 *        DEMAND DMA lets the hardware perform the necessary flow control.    *
 *      - fix read timeout for small number of sample reads in continuous     *
 *        mode.                                                               *
 *                                                                            *
 *  27  04/03/2013 D. Higgins                                                 *
 *      asm/system.h missing from / not required with 3.5 kernels             *
 *                                                                            *
 *  26  08/28/2012 D. Dubash                                                  *
 *      Free request_irq() when open call fails.                              *
 *                                                                            *
 *  25  08/22/2012 D. Dubash                                                  *
 *      When running SimWB card 0 of three cards was getting an autocal       *
 *      timeout. It appears that for some reason, when other boards are       *
 *      going through their initialization and autocal, the first card        *
 *      gets an autocal complete, however, without any local interrupt bit    *
 *      set. What is done is that the KLOOGE is skipped if autocal_pending    *
 *      is set, i.e. we are in the middle of an autocal.                      *
 *                                                                            *
 *  24  06/01/2012 D. Dubash                                                  *
 *      Fix problem where were interrupts were being disabled in KLOOGE       *
 *                                                                            *
 *  23  03/07/2012 D. Dubash                                                  *
 *      Fix O_NONBLOCK in device_read_continuous()                            *
 *                                                                            *
 *  22  02/09/2012 D. Dubash                                                  *
 *      Support for RedHawk 6.0.x                                             *
 *      Fix interrupt handler to generate IRQ_NONE if not our interrupt.      *
 *                                                                            *
 *  21  10/11/2011 D. Dubash                                                  *
 *      Provide support for the new AI64SSA/C card that replaces the old      *
 *      AI64SSA card.                                                         *
 *                                                                            *
 *  20  03/29/2011 D. Dubash                                                  *
 *      In interrupt handler, Clear DMA and Local Interrupts in case we come  *
 *      into the interrupt handler without either status being set. This will *
 *      avoid a system hang. Also, added the pci_set_master() call in case    *
 *      the BIOS does not enable bus mastering.                               *
 *                                                                            *
 *      Also fixed problem where DMA would not start after a timeout.         *
 *      Needed to set SETUP_DMA_CMD_0.                                        *
 *                                                                            *
 *  19  06/26/2009 D. Dubash                                                  *
 *      Fixed AI64LL/AI64 detection method as it was failing detection of     *
 *      an AI64 card because the card had a non-zero conversion value in it.  *
 *                                                                            *
 *  18  01/13/2009 D. Dubash                                                  *
 *      Support for RedHawk 5.2.x                                             *
 *                                                                            *
 *  17  01/29/2008 D. Dubash                                                  *
 *      Support for RedHawk 4.2.2                                             *
 *                                                                            *
 *  16  06/19/2007 D. Dubash                                                  *
 *      Added flag GFP_DMA32 to __get_free_pages so that kernel limits        *
 *      physical memory to within 4GB.                                        *
 *                                                                            *
 *  15  03/21/2007 D. Dubash                                                  *
 *      Add support for rpm.                                                  *
 *                                                                            *
 *  14  03/01/2007 D. Dubash                                                  *
 *      Use converter counter to distinguish between AI64 and AI64LL cards.   *
 *                                                                            *
 *  13  01/11/2007 D. Dubash                                                  *
 *      Distinguish between AI64LL and AI64SS cards as they have the same     *
 *      device ID and subsystem ID.                                           *
 *                                                                            *
 *  12  11/16/2006 D. Dubash                                                  *
 *      Support for RedHawk 4.1.7                                             *
 *                                                                            *
 *  11  04/26/2006 D. Dubash                                                  *
 *      Now, we do not allow certain ioctl calls when read is in progess.     *
 *      We do not allow issuing of a second ioctl call when one is already    *
 *      in progress. We do not allow as read when an ioctl call is in         *
 *      progress. Also added the write_proc() routine to allow users to       *
 *      modify the ai64_debug_mask by simply writing to the /proc/ai64 file.  *
 *                                                                            *
 *  10  03/23/2006 D. Dubash                                                  *
 *      Support for RedHawk 4.1                                               *
 *                                                                            *
 *   9  02/06/2006 D. Dubash                                                  *
 *      Support for b#=0 option. Also allow mixing reserved and allocated     *
 *      DMA buffers. Provide new ioctl's to control removing and changing     *
 *      allocated DMA buffer allocations. New ioctl's for controlling IRQ     *
 *      support.                                                              *
 *                                                                            *
 *   8  12/20/2005 D. Dubash                                                  *
 *      Get rid of sysdep.h file. Support for RH2.3.3. Also get rid of the    *
 *      PciBar structure along with local_addr and runtime_addr ioremaps      *
 *      (replaced with local_reg_address and config_reg_address respectively) *
 *                                                                            *
 *   7  08/22/2005 D. Dubash                                                  *
 *      Some bug fixes. Approach to allocate user DMA buffer. New info in     *
 *      in /proc/ai64 file.                                                   *
 *                                                                            *
 *   6  02/23/2004 D. Dubash                                                  *
 *      Fixed a bug where bad data would be returned to the user if the       *
 *      threshold size was smaller than the number of bytes read.             *
 *                                                                            *
 *   5  10/26/2004 D. Dubash                                                  *
 *      Added new IOCTL_AI64_SET_POLAR, IOCTL_AI64_VALIDATE_CHAN_0,           *
 *      IOCTL_AI64_GET_BUFFER_HWM, IOCTL_AI64_ABORT_READ  and                 *
 *      IOCTL_AI64_CTRL_AUX_SYNC_IO ioctls.                                   *
 *                                                                            *
 *   4  08/27/2004 D. Dubash                                                  *
 *      Support for 2.2 and 64 bit Opteron architecture. Added new ioctl      *
 *      to return 16bit samples instead of default 32bit samples.             *
 *                                                                            *
 *   3  07/28/04   D. Dubash                                                  *
 *      Updated to run on RedHawk 2.1. Added mmap() support. Provided new     *
 *      read/write all registers ioctl.                                       *
 *                                                                            *
 *   2  07/15/04   R. Calabro                                                 *
 *      Updated to run on RedHawk 1.4                                         *
 *                                                                            *
 *   1  Nov 2002   Evan Hillman - General Standards Corporation               *
 *      Initial release                                                       *
 *                                                                            *
 ******************************************************************************/
/***
*** pci16AI64_MAIN.C 
***
***  General description of this file:
***  	Device driver source code for General Standards PCI-16AI64
***  	64 channel PCI A/D board. This file is part of the Linux
***  	driver source distribution for this board.
***  	
***  	This file is not necessary to compile application programs, therefore 
***  	should NOT be included in binary only driver distributions.
***
***  Copyrights (c):
***  	General Standards Corporation (GSC), 2002-2003
***
***  Author:
***  	Evan Hillman, GSC Inc. (evan@generalStandards.com)
***
***  Support:
***  	Primary support for this driver is provided by GSC. 
***
***  Platform (tested on, may work with others):
***  	Linux, kernel version 2.4.x, Intel hardware.
***
***/

/* MODVERSION is redundant in RedHawk 2.1 as it is being include in the 
 * <module>.ko file. However, the <module>.o file still does not contain 
 * the module version check if MODVERSION has not been configured in the 
 * system. The Redhawk 2.1 is shipped with MODVERSION not configured in 
 * the system.
 */
#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef __VERSION__
#define __VERSION__
#endif

#ifndef MODULE
#define MODULE
#endif

/**************************************************************************
 *  If module version support is required, the modversions.h module must  *
 *  be included before any other header files. To enable versioning in    *
 *  the module, if it has been enabled in the kernel, we must first make  *
 *  sure that CONFIG_MODVERSIONS has been defined in <linux/config.h>.    *
 *  If kernel has defined conversion, we will force the driver to include *
 *  the <linux/modversions.h> file. The user could alternatively          *
 *  define the variable MODVERSIONS in the Makefile to also force         *
 *  module versions to be generated.                                      *
 **************************************************************************/
#if (defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS))
#define MODVERSIONS /* force module version on */
#endif

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
#include <generated/autoconf.h>
#ifdef MODVERSIONS
#include <config/modversions.h>
#endif
#else
#include <linux/autoconf.h>
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/pci.h>
#include <linux/mm.h>
#if 1   /* Version 2.4 on-wards - rac */
#include <linux/slab.h>
#else
#include <linux/malloc.h>
#endif
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/ioctl.h>
#include <linux/irq.h>  // Added rac
#include <linux/wait.h> // Added rac
#include <linux/signal.h>   // Added rac
#include <linux/delay.h>    // Added rac
#include <linux/proc_fs.h>

#include <asm/io.h>
// not required RH 6.3 (3.5.7 kernel)
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
#include <asm/system.h>
#endif
#include <asm/uaccess.h>
#include <asm/signal.h>
#include <asm/errno.h>

#include <linux/seq_file.h> /* procfs support */

// #include <sys/timeb.h> // Not available RH2.1

#include "ai64_ioctl.h"
#include "ai64.h"
#include "plx_regs.h"

MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("");

#define AI64 			AI64_MODULE_NAME

void ai64_abort_any_dma_inprogress(struct ai_board *ai64_device);
int ai64_fire_dma(struct ai_board *ai64_device);
void ai64_hwm_check(struct ai_board *ai64_device);

#define PATTERN                     0x1234

static char *AI64_FileName;
/**********************************************************
debug ROUTINES
***********************************************************/
#ifdef	AI64_DEBUG
#define	DbgHdr	"[%-7s %4d]:%2d.%s(): "

#define	D_ENTER			0x00000001  /* enter routine */
#define	D_EXIT			0x00000002  /* exit routine */

#define	D_L1			0x00000004  /* level 1 */
#define	D_L2			0x00000008  /* level 2 */
#define	D_L3			0x00000010  /* level 3 */
#define	D_L4			0x00000020  /* level 4 */

#define	D_ERR			0x00000040  /* level error */
#define	D_WAIT			0x00000080  /* level wait */

#define	D_INT0			0x00000100  /* interrupt level 1 */
#define	D_INT1			0x00000200  /* interrupt level 1 */
#define	D_INT2			0x00000400  /* interrupt level 2 */
#define	D_INT3			0x00000800  /* interrupt level 3 */
#define	D_INTW			0x00001000  /* interrupt wakeup level */
#define	D_INTE			0x00002000  /* interrupt error */

#define	D_RTIME			0x00010000  /* display read times */

#define D_REGS			0x00020000  /* dump registers */

#define D_IOCTL			0x00040000  /* ioctl call */

#define	D_HWM			0x00080000  /* read/write HWM */

#define	D_DATA			0x00100000  /* data level */
#define	D_DMA 			0x00200000  /* DMA level */
#define	D_PLX 			0x00400000  /* PLX Status Diagnostic */
#define	D_BUFF			0x00800000  /* Buffer Allocation */

#define	D_TIME_BTWR		0x01000000  /* display time between reads */

#define	D_NEVER			0x00000000  /* never print this debug message */
#define	D_ALWAYS		0xffffffff  /* always print this debug message */
#define	D_TEMP			D_ALWAYS    /* Only use for temporary debug code */

#define	D_ALL_LVLS		(D_L1|D_L2|D_L3|D_L4)   /* all levels */
#define	D_ALL_INTS		(D_INT0|D_INT1|D_INT2|D_INT3|D_INTW)
                                                    /* all interrupt levels */
#define	D_ALL_ERR		(D_ERR|D_INTE)  /* all error debug */
#define	D_ALL_WAIT		(D_WAIT|D_INTW) /* all wait debug */

#define	D_ALL_TIME		(D_RTIME|D_TIME_BTWR)   /* all time debug */

/* debug mask */
static int ai64_debug_mask = D_ENTER |
    D_EXIT |
    D_L1 |
    D_L2 |
    D_L3 |
    D_L4 |
    D_ERR |
    D_WAIT |
    D_INT0 | D_INT1 | D_INT2 | D_INT3 | D_INTW | D_INTE | D_RTIME |
    /*  D_REGS      | */
    D_IOCTL | D_HWM |
    /*  D_DATA      | */
    D_DMA | D_PLX | D_BUFF | D_TIME_BTWR | 0;

#define TDEBUGP(x,Str,Args...) do {                                 \
	 if (ai64_debug_mask & (x)) {  	               					\
		 if(((x) == D_ERR) || ((x) == D_INTE)) {					\
			if(ai64_device)											\
		 		printk( KERN_INFO DbgHdr "ERROR! " Str,			 	\
					AI64_FileName,__LINE__,ai64_device->minor,		\
								__FUNCTION__,Args);	                \
			else													\
		 		printk( KERN_INFO DbgHdr "ERROR! " Str,			 	\
					AI64_FileName,__LINE__,-1,						\
								__FUNCTION__,Args);	                \
		 } else {													\
			if(ai64_device)											\
		 		printk( KERN_INFO DbgHdr Str,						\
					AI64_FileName,__LINE__,ai64_device->minor,		\
								__FUNCTION__,Args);	                \
			else													\
		 		printk( KERN_INFO DbgHdr Str,						\
					AI64_FileName,__LINE__,-1,						\
								__FUNCTION__,Args);	                \
		 }															\
	 }																\
} while (0)                         
#define DEBUGPx(x,Str,Args...) do { TDEBUGP(x,Str,Args); } while (0)
#define DEBUGP(x,Str) do { TDEBUGP(x,Str "%s",""); } while (0)
#define DEBUGP_ENTER do { DEBUGP(D_ENTER,"====== ENTER ======\n"); } while (0)
#define DEBUGP_EXIT do { DEBUGP(D_EXIT,"****** EXIT  ******\n"); } while (0)

#define TIME_STAMP(What,TStamp)	{							\
	if(ai64_debug_mask & What)								\
		do_gettimeofday(&ai64_device->TStamp);				\
}

#define	Ti	ai64_device->dtime
// #define  SKIP_BTWR_BELOW 30000   /* skip display below 30 msecs */
#define	SKIP_BTWR_BELOW	0   /* skip display below 0 msecs */
#define	SKIP_READ_BELOW	0   /* skip display below 0 msecs */

#define	PRINT_TIME(What,Str,T_Start,T_End,T_Hwm,Bytes,Skip_Below)			\
	do { 																	\
		if(ai64_debug_mask & What)	{										\
			Ti = (ai64_device->T_End.tv_sec - ai64_device->T_Start.tv_sec)*1000000 +	\
				 (ai64_device->T_End.tv_usec - ai64_device->T_Start.tv_usec);\
			if(Ti > 0xf0000000)	/* KLOOGE FOR BAD TIME */					\
				break;														\
			if(ai64_device->T_Hwm < Ti) 									\
				ai64_device->T_Hwm = Ti;									\
			if(Ti < Skip_Below) break;										\
			if(Bytes > 0) 													\
				DEBUGPx(What,Str "Time = %5u.%03u ms, %2u.%04u MB/s "  		\
					"(HWM=%5u.%03u ms)\n", 									\
					Ti/1000,Ti%1000,Bytes/Ti,((Bytes%Ti)*10000)/Ti,			\
					ai64_device->T_Hwm/1000, ai64_device->T_Hwm%1000);		\
			else 															\
				DEBUGPx(What,Str "Time = %5u.%03u ms "						\
					"(HWM=%5u.%03u ms)\n", 									\
					Ti/1000,Ti%1000, 										\
					ai64_device->T_Hwm/1000, ai64_device->T_Hwm%1000);		\
		}																	\
	} while (0)

#define REG_DUMP(Lvl, Dev, Desc)  ai64_reg_dump(Lvl, Dev, Desc)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
module_param(ai64_debug_mask, int, 0644);
#else
MODULE_PARM(ai64_debug_mask, "i");
#endif
MODULE_PARM_DESC(ai64_debug_mask, "AI64 Debug Mask");

#else                           /* else AI64_DEBUG */

#define DEBUGPx(x,Str,Args...)
#define DEBUGP(x,Str)
#define	DEBUGP_ENTER
#define	DEBUGP_EXIT
#define	TIME_STAMP(What,TStamp)
#define	PRINT_TIME(What,Str,T_Start,T_End,T_Hwm,Bytes,Skip_Below)
#define REG_DUMP(Lvl, Dev, Desc)
#endif                          /* end AI64_DEBUG */

/**********************************************************
error ROUTINES
***********************************************************/
#define	ErrHdr	"[%-7s %4d]:%2d.%s(): ERROR!!!: "

#define TERRP(Str,Args...)                                      \
     do {                                                       \
		if(ai64_device)											\
		 	printk( KERN_INFO ErrHdr Str,						\
				AI64_FileName,__LINE__,ai64_device->minor,		\
				__FUNCTION__,Args);				                \
		else													\
		 	printk( KERN_INFO ErrHdr Str,						\
				AI64_FileName,__LINE__,-1,						\
				__FUNCTION__,Args);				                \
} while (0)                         

#define ERRPx(Str,Args...) do { TERRP(Str,Args); } while (0)
#define ERRP(Str) do { TERRP(Str "%s",""); } while (0)


// MODULE_LICENSE("GPL");

static int resmem[2] = { 0 };   /* start, size */

#define	RESMEM_START	resmem[0]
#define	RESMEM_SIZE		resmem[1]

#define DNB DEF_NUM_DMA_BUFFERS

/* number of buffers per board */
static int b0 = DNB, b1 = DNB, b2 = DNB, b3 = DNB, b4 = DNB, b5 = DNB, b6 = DNB, b7 =
    DNB, b8 = DNB, b9 = DNB;
static int *nbufs[] = {
    &b0, &b1, &b2, &b3, &b4, &b5, &b6, &b7, &b8, &b9
};

static int resmem_size = 2;  /* number of uints in 'resmem' */
module_param_array(resmem, uint, &resmem_size, 0);
MODULE_PARM_DESC(resmem, "Reserved Memory: start address, size");
module_param(b0, uint, 0);   /* board 0, number of buffers */
module_param(b1, uint, 0);   /* board 1, number of buffers */
module_param(b2, uint, 0);   /* board 2, number of buffers */
module_param(b3, uint, 0);   /* board 3, number of buffers */
module_param(b4, uint, 0);   /* board 4, number of buffers */
module_param(b5, uint, 0);   /* board 5, number of buffers */
module_param(b6, uint, 0);   /* board 6, number of buffers */
module_param(b7, uint, 0);   /* board 7, number of buffers */
module_param(b8, uint, 0);   /* board 8, number of buffers */
module_param(b9, uint, 0);   /* board 9, number of buffers */
MODULE_PARM_DESC(b0, "Board 0: number of buffers");
MODULE_PARM_DESC(b1, "Board 1: number of buffers");
MODULE_PARM_DESC(b2, "Board 2: number of buffers");
MODULE_PARM_DESC(b3, "Board 3: number of buffers");
MODULE_PARM_DESC(b4, "Board 4: number of buffers");
MODULE_PARM_DESC(b5, "Board 5: number of buffers");
MODULE_PARM_DESC(b6, "Board 6: number of buffers");
MODULE_PARM_DESC(b7, "Board 7: number of buffers");
MODULE_PARM_DESC(b8, "Board 8: number of buffers");
MODULE_PARM_DESC(b9, "Board 9: number of buffers");

struct board_entry boards_supported[] = {
    /*    subvendor    subsystem name       index ID */
    {PCI_DEVICE_ID_PLX_9080, GSC_SUBVENDOR, 0x2406, "PMC-12AI64", gsc12ai64},   
													/** PMC-12AI64 */
    {PCI_DEVICE_ID_PLX_9080, GSC_SUBVENDOR, 0x2407, "PMC-16AI64", gsc16ai64},   
													/** PMC-16AI64 */
    {PCI_DEVICE_ID_PLX_9080, GSC_SUBVENDOR, 0x2868, "PMC-16AI64SS", gsc16ai64ss},
                                                    /** PCM-16AI64SS */
    {PCI_DEVICE_ID_PLX_9056, GSC_SUBVENDOR, 0x3101, "PMC-16AI64SSA/C", gsc16ai64ssa_c},
                                                    /** PCM-16AI64SSA_C */
    {0, 0, 0, "null"}  /* null terminated list.... */
};

/* Driver Version */
static const char device_version[] =
    "source version - " AI64_VERSION " built - "
    "$Date: 2007/03/21 13:17:43 $";

/* module load/unload functions */
static int ai64_init(void);
static void ai64_exit(void);

/* local functions */
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static int device_open(struct inode *, struct file *);
static int device_close(struct inode *, struct file *);
static int device_read_buffer(struct ai_board *ai64_device, char *buf,
                              int nsamp, int noblock);
static int device_read_continuous(struct ai_board *ai64_device, char *buf,
                                  int nsamp, int noblock);

static int device_read_dma(struct ai_board *ai64_device, int nsamples,
                           int DMAMode);
int ai64_configure_mem_regions(struct ai_board *ai64_device,
                               struct pci_dev *pdev, int which,
                               u32 * address, u32 * size, u32 * flags);
void ai64_deconfigure_mem_regions(struct ai_board *ai64_device, int which,
                                  u32 address, u32 size, u32 flags);
int ai64_allocate_physical_memory(struct ai_board *ai64_device, int size);
void ai64_free_physical_memory(struct ai_board *ai64_device);
int ai64_validate_data(struct ai_board *ai64_device);

/* local module data */
static struct file_operations device_fops = {
  open:device_open,
  release:device_close,
  read:device_read,
  write:device_write,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
  unlocked_ioctl:unlocked_device_ioctl,
#else
  ioctl:device_ioctl,
#endif
  mmap:device_mmap,
};

static __s8 version[8];
static __s8 built[32];

static int proc_enabled;
static int num_boards;
static int device_major = 0;    // device major number (dynamically assigned)
static struct ai_board *boards = 0; // linked list of all pci16AI64 boards found
static struct ai_board *ai64_device_list[MAX_BOARDS];


#ifdef DEBUG
unsigned short reg;
uint int_other_count[MAX_BOARDS];
uint int_count[MAX_BOARDS];
uint dma_count[MAX_BOARDS];
uint channel_irq[MAX_BOARDS];
uint channel_expected[MAX_BOARDS];
int board_type[MAX_BOARDS];
void *context[MAX_BOARDS];

unsigned long long start_time = 0;
unsigned long long last_time = 0;
unsigned long long now_time = 0;
#define Read_TSC(val) __asm__ __volatile__("rdtsc" : "=A" (val))
#define Ticks (2392.075)    // ticks per microsecond (from /proc/cpuinfo)
#endif

void ai64_vmops_open(struct vm_area_struct *vma)
{
    struct ai_board *ai64_device =(struct ai_board *)vma->vm_file->private_data;

    ai64_device = ai64_device; /* get rid of warning */

    DEBUGPx(D_L2, "end= 0x%lx start=0x%lx size=0x%lx\n",
            vma->vm_end, vma->vm_start, (vma->vm_end-vma->vm_start));
}

void ai64_vmops_close(struct vm_area_struct *vma)
{
    unsigned int size;
    struct ai_board *ai64_device =(struct ai_board *)vma->vm_file->private_data;

    DEBUGPx(D_L2, "end= 0x%lx start=0x%lx size=0x%lx\n",
            vma->vm_end, vma->vm_start, (vma->vm_end-vma->vm_start));

    size = vma->vm_end-vma->vm_start;

    ai64_device->phys_mem_size_freed += size;

    /* if all of the mapped memory is freed, we can go and clean up
     * the buffer
     */
    if(ai64_device->phys_mem_size_freed == ai64_device->phys_mem_size) {
        /* free physical memory */
        DEBUGP(D_L2, "##### FREEING MEMORY ######\n");
        ai64_free_physical_memory(ai64_device);
    }
}

struct vm_operations_struct vm_ops = {
    open:   ai64_vmops_open,
    close:  ai64_vmops_close
};

void ai64_reg_dump(int dbglvl, struct ai_board *ai64_device, char *desc)
{
    int q;

    DEBUGPx(dbglvl, "ai64%d: %s\n", ai64_device->minor, desc);
    for (q = 0; q < 9; q++) {
        if (q == 2)
            continue;   // Skip FIFO read to keep it coherent
        DEBUGPx(dbglvl, "@ 0x%02x = 0x%.8x\n", q * 4,
                readlocal(ai64_device, q));
    }
    DEBUGPx(dbglvl, "PLX IRQ = 0x%.8x.\n",
            readl(IntCntrlStat(ai64_device)));
}

/************************************************************************/
/* ai64_timeout_handler                                                 */
/************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
void ai64_timeout_handler(unsigned long ptr)
#else
void ai64_timeout_handler(struct timer_list* ptr)
#endif
{
    struct ai_board *ai64_device = (struct ai_board *) ptr;

    DEBUGP_ENTER;
    DEBUGP(D_ERR, "ERROR - device timeout\n");
    DEBUGPx(D_L1, "### CMDS=0x%x ICR=0x%x ICSR=0x%x\n",
            readl(DMACmdStatus(ai64_device)),
            readl(IntCtrlReg(ai64_device)),
            readl(IntCntrlStat(ai64_device)));
    DEBUGPx(D_L1, "### BCR=0x%x SSCR=0x%x\n",
            readl(BoardCtrlReg(ai64_device)),
            readl(ScanSynchCtrlReg(ai64_device)));
    DEBUGPx(D_L1, "### GEN: A=%8.8X B=%8.8X\n",
            readl(RateAGenReg(ai64_device)),
            readl(RateBGenReg(ai64_device)));

    ai64_device->timeout = TRUE;

    DEBUGPx(D_L1,
            "timeout -> plx int %.8X board control %.8X local int %.8X "
            "DMA status %.8X buffer size: %.8X \n",
            readl(IntCntrlStat(ai64_device)), readlocal(ai64_device,
                                                        AI64_BOARD_CTRL_REG),
            readlocal(ai64_device, AI64_INT_CTRL_REG),
            readl(DMACmdStatus(ai64_device)), readlocal(ai64_device,
                                                        AI64_BUFFER_SIZE_REG));
    DEBUGPx(D_L1, "DMA Mode %.8X\n", readl(DMAMode0(ai64_device)));
    DEBUGPx(D_L1, "Firm rev %.8X\n", readlocal(ai64_device, AI64_BOARD_CONFIG_REG));

    DEBUGPx(D_L1, "    init_pending %d\n", ai64_device->init_pending);
    DEBUGPx(D_L1, " autocal_pending %d\n", ai64_device->autocal_pending);
    DEBUGPx(D_L1, "   lo_hi_pending %d\n", ai64_device->lo_hi_pending);
    DEBUGPx(D_L1, " chready_pending %d\n", ai64_device->chready_pending);
    DEBUGPx(D_L1, "   dmainprogress %d\n", ai64_device->dmainprogress);

    /* wake up whatever was waiting. */

    if (ai64_device->init_pending) {
        ai64_device->init_pending = FALSE;
        DEBUGP(D_WAIT, "Wakeup ioctlwq\n");
        wake_up(&ai64_device->ioctlwq);
    }

    if (ai64_device->autocal_pending) {
        ai64_device->autocal_pending = FALSE;
        DEBUGP(D_WAIT, "Wakeup ioctlwq\n");
        wake_up(&ai64_device->ioctlwq);
    }

    if (ai64_device->chready_pending) {
        ai64_device->chready_pending = FALSE;
        ai64_device->wakeup_dma_pending = FALSE;
        DEBUGP(D_WAIT, "Wakeup dmawq\n");
        wake_up(&ai64_device->dmawq);
    }

    if (ai64_device->lo_hi_pending) {
        DEBUGP(D_WAIT, "Timeout:  waiting for LO/HI Pending FALSE...\n");
        ai64_device->lo_hi_pending = FALSE;
        ai64_device->wakeup_lohi_pending = FALSE;
        DEBUGP(D_WAIT, "Wakeup lohiwq\n");
        wake_up(&ai64_device->lohiwq);
    }

    if (ai64_device->dmainprogress) {
        ai64_device->dmainprogress = NO_DMA_INPROGRESS;
        ai64_device->wakeup_dma_pending = FALSE;
        DEBUGP(D_WAIT, "Timeout:  waiting for DMA Complete Wakeup ...\n");
        DEBUGP(D_WAIT, "Wakeup dmawq\n");
        wake_up(&ai64_device->dmawq);
    }

    if (ai64_device->wakeup_dma_pending) {
        ai64_device->wakeup_dma_pending = FALSE;
        DEBUGP(D_WAIT, "Wakeup dmawq\n");
        wake_up(&ai64_device->dmawq);
        ai64_device->init_pending++;
    }

    DEBUGP_EXIT;
}

static int ai64_proc_show(struct seq_file *m, void *v);
ssize_t    ai64_proc_write(struct file *file, const char __user *buf,
                           size_t count, loff_t *data);

static int ai64_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, ai64_proc_show, NULL);
}

#if   	 LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
static struct proc_ops ai64_proc_operations = {
        .proc_open           = ai64_proc_open,
        .proc_read           = seq_read,
        .proc_write          = ai64_proc_write,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static struct file_operations ai64_proc_operations = {
        .owner          = THIS_MODULE,
        .open           = ai64_proc_open,
        .read           = seq_read,
        .write          = ai64_proc_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

/* procfs support */
static int ai64_proc_show(struct seq_file *m, void *v)
{
    int             bcnt;
    struct ai_board *ai64_device = 0;
    char            dma_none[50];
    char            dma_reserved[50];
    char            dma_allocated[50];
    char            *pn, *pr, *pa;

    seq_printf(m, "version: %s\n"
                  "built: %s\n", AI64_VERSION, built);

#ifdef	AI64_DEBUG
    seq_printf(m, "debug mask: 0x%08x\n",ai64_debug_mask);
#endif

    seq_printf(m, "boards: %d\n", num_boards);

    dma_none[0] = dma_reserved[0] = dma_allocated[0] = 0;
    pn = dma_none;
    pr = dma_reserved;
    pa = dma_allocated;

    for (bcnt = 0; bcnt < num_boards; bcnt++) {
        ai64_device = ai64_device_list[bcnt];
        seq_printf(m, "  card=%d: [%02x:%02x.%1d] bus=%d, slot=%d, func=%d, "
                    "irq=%d, nbuf=%d, Firmware=0x%x (%s)\n",
                    bcnt, ai64_device->bus, ai64_device->slot,
                    ai64_device->func, ai64_device->bus, ai64_device->slot,
                    ai64_device->func, ai64_device->irqlevel,
                    ai64_device->num_dma_buffers, ai64_device->firmware,
					boards_supported[ai64_device->board_type].name);

        if(ai64_device->num_dma_buffers == 0) {
            sprintf(pn,"%d ",bcnt);
            pn += strlen(pn);
        } else {
            if(ai64_device->use_resmem) {
                sprintf(pr,"%d ",bcnt);
                pr += strlen(pr);
            } else {
                sprintf(pa,"%d ",bcnt);
                pa += strlen(pa);
            }
        }
    }

    seq_puts(m,"\n");

    if(dma_none[0] != 0) {
        seq_printf(m,"DMA Memory: (NONE)     : card number(s): %s\n",
                                                            dma_none);
    }
    if(dma_allocated[0] != 0) {
        seq_printf(m,"DMA Memory: (Allocated): card number(s): %s\n",
                                                            dma_allocated);
    }

    if(dma_reserved[0] != 0) {
        seq_printf(m,"DMA Memory: (Reserved) : card number(s): %s\n", 
                                                            dma_reserved);

        seq_printf(m,"                       : Start=0x%x, Size=0x%x\n",
                                                RESMEM_START, RESMEM_SIZE);
    }

#ifdef DEBUG
    seq_puts(m, "Debug Stats:\n ");
    seq_puts(m,
           "type\tother_ints\ttotal_ints\tdma_ints\tchan_irq\tchan_expect\n");

    for (j = 0; j < num_boards; j++) {
        seq_printf(m, "%d\t\t%ld\t\t%ld\t\t%ld\t\t%ld\t\t%ld\t\t%p\n",
                board_type[j],
                int_other_count[j],
                int_count[j],
                dma_count[j],
                channel_irq[j], channel_expected[j], context[j]
            );
    }

#endif

    return 0;
}

#if 0
/******************************************************************************
*
*	Function:	read_proc
*
*	Purpose:
*
*		Implement the read service for the module's /proc file system
*		entry. Read the documentation on this service for details, as
*		we ignore some arguments according to our needs and the
*		documentation.
*
*	Arguments:
*
*		page	The data produced is put here.
*
*		start	Records pointer to where the data in "page" begins.
*
*		offset	The offset into the file where we're to begin.
*
*		count	The limit to the amount of data we can produce.
*
*		eof	Set this flag when we hit EOF.
*
*		data	A private data item that we may use, but don't.
*
*	Returned:
*
*		int	The number of characters written.
*
******************************************************************************/
static int read_proc(char *page,
                     char **start,
                     off_t offset, int count, int *eof, void *data)
{
    char *cp;
    int bcnt;
    struct ai_board *ai64_device = 0;
    char dma_none[50];
    char dma_reserved[50];
    char dma_allocated[50];
    char *pn, *pr, *pa;
    

#define	_PROC_MSG_SIZE	128

#if PAGE_SIZE < _PROC_MSG_SIZE
#error	"/proc file size may be too big."
#endif
    int i;
#ifdef DEBUG
    int j;
    char str[256];
#endif

    if (!try_module_get(THIS_MODULE)) {
        ERRP("Unable to increment the module count\n");
        return -EBUSY;
    }
    i = sprintf(page,
                "version: %s\n"
                "built: %s\n", AI64_VERSION, built);

    cp = &page[i];

#ifdef	AI64_DEBUG
    i = sprintf(cp, "debug mask: 0x%08x\n",ai64_debug_mask);
    cp += i;
#endif

    i = sprintf(cp, "boards: %d\n", num_boards);
    cp += i;

    dma_none[0] = dma_reserved[0] = dma_allocated[0] = 0;
    pn = dma_none;
    pr = dma_reserved;
    pa = dma_allocated;

    for (bcnt = 0; bcnt < num_boards; bcnt++) {
        ai64_device = ai64_device_list[bcnt];
        i = sprintf(cp, "  card=%d: [%02x:%02x.%1d] bus=%d, slot=%d, func=%d, "
                    "irq=%d, nbuf=%d, Firmware=0x%x (%s)\n",
                    bcnt, ai64_device->bus, ai64_device->slot,
                    ai64_device->func, ai64_device->bus, ai64_device->slot,
                    ai64_device->func, ai64_device->irqlevel,
                    ai64_device->num_dma_buffers, ai64_device->firmware,
					boards_supported[ai64_device->board_type].name);
        cp += i;

        if(ai64_device->num_dma_buffers == 0) {
            sprintf(pn,"%d ",bcnt);
            pn += strlen(pn);
        } else {
            if(ai64_device->use_resmem) {
                sprintf(pr,"%d ",bcnt);
                pr += strlen(pr);
            } else {
                sprintf(pa,"%d ",bcnt);
                pa += strlen(pa);
            }
        }
    }

    i = sprintf(cp,"\n");
    cp += i;

    if(dma_none[0] != 0) {
        i = sprintf(cp,"DMA Memory: (NONE)     : card number(s): %s\n",
                                                            dma_none);
        cp += i;
    }
    if(dma_allocated[0] != 0) {
        i = sprintf(cp,"DMA Memory: (Allocated): card number(s): %s\n",
                                                            dma_allocated);
        cp += i;
    }

    if(dma_reserved[0] != 0) {
        i = sprintf(cp,"DMA Memory: (Reserved) : card number(s): %s\n", 
                                                            dma_reserved);
        cp += i;

        i = sprintf(cp,"                       : Start=0x%x, Size=0x%x\n",
                                                RESMEM_START, RESMEM_SIZE);
        cp += i;
    }

    *cp = 0;

//  page[i] = 0;

#ifdef DEBUG
    strcat(page, "Debug Stats:\n ");
    strcat(page,
           "type\tother_ints\ttotal_ints\tdma_ints\tchan_irq\tchan_expect\n");

    for (j = 0; j < num_boards; j++) {
        sprintf(str, "%d\t\t%ld\t\t%ld\t\t%ld\t\t%ld\t\t%ld\t\t%p\n",
                board_type[j],
                int_other_count[j],
                int_count[j],
                dma_count[j],
                channel_irq[j], channel_expected[j], context[j]
            );
        strcat(page, str);
    }

#endif
    //strcat(page, "\n");

    i = strlen(page) + 1;

    if (i >= PAGE_SIZE) {
        DEBUGP(D_ERR, "/proc/xxx is larger than PAGE_SIZE\n");
        i = PAGE_SIZE - 1;
    }

    i--;
    eof[0] = 1;
    module_put(THIS_MODULE);
    return (i);
}
#endif

ssize_t    ai64_proc_write(struct file *file, const char __user *buf,
                           size_t count, loff_t *data)
{
    struct ai_board *ai64_device = ai64_device_list[0];
    ssize_t ret = -ENOMEM;
    char *dbgm="ai64_debug_mask";
    char *page;
    char *endp, *p;
    int  value;

    if(count > PAGE_SIZE)
        return -EOVERFLOW;

    page = (char *)__get_free_page(GFP_KERNEL);
    if (page) {
        ret = -EFAULT;
        if (copy_from_user(page, buf, count))
            goto out;
        page[count-1]=0;
        printk(KERN_INFO AI64_DEVICE_NAME "input string:[%s] size:[%ld]\n",
                    page, (unsigned long)count);
		 		
        if(strncmp(page,dbgm,strlen(dbgm)) == 0) {
            p = page+strlen(dbgm);
            while(*p == ' ')p++;    /* leading blanks */
            if(*p == '=')p++;       /* skip '=' */
            while(*p == ' ')p++;    /* following blanks */
            value = simple_strtol(p,&endp,0);
            if(*endp && (*endp != ' ')) {
                goto out_inval;
            }

/* for now, we only use this routine to change the debug mask */
#ifdef AI64_DEBUG
            ai64_debug_mask=value;
            printk(KERN_INFO AI64_DEVICE_NAME "ai64_debug_mask=0x%08x (%d)\n",
                    ai64_debug_mask, ai64_debug_mask);
#else  /* AI64_DEBUG */
            printk(KERN_INFO AI64_DEVICE_NAME ": Debugging not enabled!\n");
#endif /* AI64_DEBUG */

            ret = count;
            goto out;
        }
                
out_inval:
        ERRPx("Invalid argument: [%s]\n",page);
        ret = -EINVAL;
    }
out:
    free_page((unsigned long)page);
    return ret;
}

#if 0
/******************************************************************************
*
*	Function:	proc_get_info
*
*	Purpose:
*
*		Implement the get_info() service for /proc file system support.
*
*	Arguments:
*
*		page	The data produced is put here.
*
*		start	Records pointer to where the data in "page" begins.
*
*		offset	The offset into the file where we're to begin.
*
*		count	The limit to the amount of data we can produce.
*
*		dummy	A parameter that is unused (for kernel 2.2 only).
*
*	Returned:
*
*		int	The number of characters written.
*
******************************************************************************/
int proc_get_info(char *page,
                  char **start, off_t offset, int count, int dummy)
{
    int eof;
    int i;

    i = read_proc(page, start, offset, count, &eof, NULL);
    return (i);
}
#endif

void ai64_locate_memory_range(void **kernel_top, void **processor_physmem);
int ai64_check_args(void *kernel_top, void *processor_physmem, int board_count);

/************************************************************************/
/* module initalization: detect card(s) and set up descriptor(s)        */
/************************************************************************/
static int ai64_init(void)
{
    unsigned long   regval;
    struct          ai_board *ai64_device = 0, *device_next = 0;
    struct          pci_dev *pdev = NULL;
    int             index = 0;
    int             i, j;
    int             board_type;
    int             board_count;
    int             found;
    int             region_error;
    int             slash = '/';
    int             ResourceCount;
    static void     *kernel_top;
    static void     *processor_physmem;
    int             resmem_specified;
    char            str[50];
    char            fstr[30];
	int 			not_ai64_board;
    int             subsys_id;
	int				masterclk;

    AI64_FileName = strrchr(__FILE__, slash);
    if (AI64_FileName[0] == '/')    /* bump past slash */
        AI64_FileName++;

    DEBUGP_ENTER;
    DEBUGPx(D_L1, "AI64_FileName=%s\n", AI64_FileName);

    strcpy(version, AI64_VERSION);
    sprintf(built, "%s, %s", __DATE__, __TIME__);
    DEBUGPx(D_L1, "driver version %s - built %s\n", version, built);

    /* for physical memory allocation, both RESMEM_START and RESEMEM_SIZE
     * must be specified. For drive to allocate DMA memory, either set
    * both parameters to zero or do not specify the resmem= command. */
    if (RESMEM_START && RESMEM_SIZE)
        resmem_specified = 1;
    else {
        RESMEM_START = RESMEM_SIZE = 0;
        resmem_specified = 0;
    }

    /* first, find number of boards that are found */
    pdev = NULL;        /* start from top */
    board_count=0;      /* initialize board count */
    while ((pdev =
//          pci_find_device(PCI_VENDOR_ID_PLX, PCI_DEVICE_ID_PLX_9080,
//          pci_get_device(PCI_VENDOR_ID_PLX, PCI_DEVICE_ID_PLX_9080,
            pci_get_device(PCI_VENDOR_ID_PLX, PCI_ANY_ID,
                            pdev))) {

		if(!((pdev->device == PCI_DEVICE_ID_PLX_9080) ||
			(pdev->device == PCI_DEVICE_ID_PLX_9056))) {
			continue;
		}

        if (pci_enable_device(pdev))
            continue;

        /* enable DMA master bit in case the BIOS does not enable it */
        pci_set_master(pdev); 

        /* determine if this is one of the boards supported. */
        found = FALSE;
        board_type = 0;
        while (boards_supported[board_type].subsystem_device != 0) {
            if ((boards_supported[board_type].subsystem_device ==
                 								pdev->subsystem_device)
                && (boards_supported[board_type].subsystem_vendor ==
                    pdev->subsystem_vendor)
				&& (boards_supported[board_type].device == pdev->device)) {
                board_count++;
                break;
            }
            board_type++;
        }
    }

    DEBUGPx(D_L1,"Number of AI64 boards found = %d\n",board_count);

    if(board_count == 0) {
        ERRP("No device found\n");
        DEBUGP_EXIT;
        return (-ENODEV);
    }

    /* next, check if reserved memory is specified */
    if(resmem_specified == 1) { /* if physical memory specified */
        /* locate top of kernel memory and top of physical cpu memory */
        ai64_locate_memory_range(&kernel_top, &processor_physmem);

        DEBUGPx(D_L1, "kernel_top=0x%p processor_physmem=0x%p\n",
            kernel_top, processor_physmem);

        /* validate arguments against specified resmem range */
        if (ai64_check_args(kernel_top, processor_physmem, board_count)) {
            DEBUGP_EXIT;
            return (-EINVAL);
        }
    
        DEBUGPx(D_L1, "resmem[0]=0x%x resmem[1]=0x%x\n", RESMEM_START,
                RESMEM_SIZE);
    } else {
        kernel_top = processor_physmem = NULL;
    }

    /***********************************************************
     *    if nbufs=  0, no buffers to be allocated,
     *    if nbufs=  #, # of buffers to be allocated or reserved,
     *    if nbufs= -#, allocate (not reserve) number of buffers
     */
    for (i = 0; i < MAX_BOARDS; i++) {
        sprintf(fstr,(i <= (board_count-1)) ? "### Found     ###":
                                              "=== Not Found ===");

        if((*nbufs[i] == 0) || (i >= board_count))  {
            sprintf(str,"No buffers");
        }
        else if (*nbufs[i] > 0) {   /* greater than 0 */
            if(RESMEM_SIZE) {
                sprintf(str,"Use Reserved Memory");
            } else {
                sprintf(str,"Memory to be Allocated");
            }
        }
        else {  /* less than zero */
            sprintf(str,"Memory to be Allocated");
        }
            
        DEBUGPx(D_L1, "Number of buffers: board_%d=%2d (%s: %s)\n", i, 
                           (int)abs(*nbufs[i]), fstr, str);
    }

    pdev = NULL;        /* start from top */
    while ((pdev =
//          pci_find_device(PCI_VENDOR_ID_PLX, PCI_DEVICE_ID_PLX_9080,
//          pci_get_device(PCI_VENDOR_ID_PLX, PCI_DEVICE_ID_PLX_9080,
            pci_get_device(PCI_VENDOR_ID_PLX, PCI_ANY_ID,
                            pdev))) {

		if(!((pdev->device == PCI_DEVICE_ID_PLX_9080) ||
			(pdev->device == PCI_DEVICE_ID_PLX_9056))) {
			continue;
		}

        if (pci_enable_device(pdev))
            continue;

        /* determine if this is one of the boards supported. */
        found = FALSE;
        board_type = 0;
        while (boards_supported[board_type].subsystem_device != 0) {
            if ((boards_supported[board_type].subsystem_device ==
                 						pdev->subsystem_device)
                && (boards_supported[board_type].subsystem_vendor ==
                    pdev->subsystem_vendor)
				&& (boards_supported[board_type].device == pdev->device)) {
                found = TRUE;
                DEBUGPx(D_L2, "found board %s type %d\n",
                        boards_supported[board_type].name, board_type);
            }
            if (found)
                break;
            board_type++;
        }

        if (found) {
#ifdef DEBUG
            pci_read_config_word(pdev, 0x2E, &reg);
            DEBUGPx(D_L2, ": config reg=0x%x\n", reg);
#endif
            ai64_device =
                (struct ai_board *) kmalloc(sizeof(struct ai_board),
                                            GFP_KERNEL);
            if (!ai64_device) {
                ERRPx("(%d): Unable to allocate memory\n", index);
                continue;
            }
            memset(ai64_device, 0, sizeof(struct ai_board));/* zero structure */
            ai64_device->minor = index; /* set early for DEBUG error messages */
            DEBUGPx(D_L1,
                    "attaching board #%d device pointer 0x%p\n", index + 1,
                    ai64_device);
            ai64_device->board_type = board_type;

            /***********************************************************
             *    if nbufs=  0, no buffers to be allocated,
             *    if nbufs=  #, # of buffers to be allocated or reserved,
             *    if nbufs= -#, allocate (not reserve) number of buffers
             */
            ai64_device->num_dma_buffers = abs(*nbufs[index]);

            /* if user has specified a reserved memory and buffer count */
            if (RESMEM_START && (*nbufs[index] > 0))
                ai64_device->use_resmem = 1;    /* use reserved memory */
            else
                ai64_device->use_resmem = 0;    /* do not use reserved mem */

            /***/
            /*
             * The bars may be either in I/O space or memory space.  Figure 
             * out which this board is and Deal with either.
             */
            ResourceCount = 0;

            ai64_device->dma = NULL;
            /* if DMA buffers specified */
            if(ai64_device->num_dma_buffers) {
                ai64_device->dma =
                    (struct ai_dma *) kmalloc(ai64_device->num_dma_buffers *
                                          sizeof(struct ai_dma),
                                          GFP_KERNEL);
                if (!ai64_device->dma) {
                    ERRP("Unable to allocate memory\n");
                    kfree(ai64_device);
                    ai64_device=NULL;
                    continue;
                }

                memset(ai64_device->dma, 0, ai64_device->num_dma_buffers * sizeof(struct ai_dma));  /* zero structure */
            }

            /* pre assign physical memory buffers if resmem is being used */
            if (ai64_device->use_resmem) {
                for (i = 0; i < ai64_device->num_dma_buffers; i++) {
                    /* physical memory address */
                    ai64_device->dma[i].dmaoffset =
                        (uintptr_t) RESMEM_START;
                    RESMEM_START += DMA_SIZE;
                }
            }

            region_error = 0;   /* set success flag */
            /*** mark all the memory regions in use ***/
            for (i = 0; i < AI64_MAX_REGION; i++) {
                if (ai64_configure_mem_regions(ai64_device, pdev, i,
                                               &ai64_device->mem_region[i].
                                               address,
                                               &ai64_device->mem_region[i].
                                               size,
                                               &ai64_device->mem_region[i].
                                               flags)) {
                    DEBUGP(D_ERR,
                            "#### ai64_configure_mem_regions FAILED\n");
                    /*** If configuration failed, we need to remove the
					 *** memory requested
					 ***/
                    /* remove requested memory regions */
                    for (j = 0; j < AI64_MAX_REGION; j++) {
                        ai64_deconfigure_mem_regions(ai64_device, j,
                                                     ai64_device->
                                                     mem_region[j].address,
                                                     ai64_device->
                                                     mem_region[j].size,
                                                     ai64_device->
                                                     mem_region[j].flags);
                    }

                    /* re-adjust resmem start */
                    if (ai64_device->use_resmem) {
                        RESMEM_START -=
                            (ai64_device->num_dma_buffers * DMA_SIZE);
                    }

                    if(ai64_device->dma)
                        kfree(ai64_device->dma);
                    kfree(ai64_device);
                    ai64_device=NULL;
#if 0   /* could be an AI64LL card */
                    ERRPx("PCI Bus# %d, Device# %d.%d: Failed!!!\n",
                          pdev->bus->number,
                          PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));
#endif
                    region_error++; /* mark error */
                    break;
                }
            }

            if (region_error)   /* if region failed, skip card */
                continue;

            /*** remap physical to virtual ***/
            /*** PCI_BASE_ADDRESS_0 ***/
            ai64_device->config_reg_address =
#if   	 LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
                (unsigned int *) ioremap(ai64_device->
                                                 mem_region
                                                 [CONFIG_REGION].address,
                                                 ai64_device->
                                                 mem_region
                                                 [CONFIG_REGION].size);
#else
                (unsigned int *) ioremap_nocache(ai64_device->
                                                 mem_region
                                                 [CONFIG_REGION].address,
                                                 ai64_device->
                                                 mem_region
                                                 [CONFIG_REGION].size);
#endif

            /*** PCI_BASE_ADDRESS_2 ***/
            ai64_device->local_reg_address =
#if   	 LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
                (unsigned int *) ioremap(ai64_device->
                                                 mem_region[LOCAL_REGION].
                                                 address,
                                                 ai64_device->
                                                 mem_region[LOCAL_REGION].
                                                 size);
#else
                (unsigned int *) ioremap_nocache(ai64_device->
                                                 mem_region[LOCAL_REGION].
                                                 address,
                                                 ai64_device->
                                                 mem_region[LOCAL_REGION].
                                                 size);
#endif

            regval = readlocal(ai64_device, AI64_BOARD_CONFIG_REG);
            /* if firmware value is bad, error out */
            if (regval == 0xffffffff) {
                /* Unmap the memory space allocated for the device found */
                iounmap(ai64_device->local_reg_address);
                iounmap(ai64_device->config_reg_address);

                /* remove requested memory regions */
                for (i = 0; i < AI64_MAX_REGION; i++) {
                    ai64_deconfigure_mem_regions(ai64_device, i,
                                                 ai64_device->
                                                 mem_region[i].address,
                                                 ai64_device->
                                                 mem_region[i].size,
                                                 ai64_device->
                                                 mem_region[i].flags);
                }
                if(ai64_device->dma)
                    kfree(ai64_device->dma);
                kfree(ai64_device);
                ai64_device=NULL;

                ERRPx("PCI Bus# %d, Device# %d.%d: Failed!!!\n",
                      pdev->bus->number,
                      PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));
                continue;
            }

            /* Now, since AI64 and AI64LL cards have the SAME VENDOR_ID
             * (0x10b5), DEVICE_ID  (0x9080) and SUBSYSTEM_ID (0x2407)
             * we need to distinguish between the two cards. This is 
			 * accomplished by checking if a conversion counter register
             * once enabled is incrementing on the AI64LL. This feature does
			 * not exist on the AI64 card.
			 */

			#define SC_AIN_RESET_INPUTS_MASK    0x00040000
			#define SC_AIN_ENABLE_SCAN_MASK     0x00000020
			#define CONV_COUNTER                6

			/* disable the incrementing of the conversion counter */
			writelocal(ai64_device, SC_AIN_RESET_INPUTS_MASK, 
                                            AI64_SCAN_SYNCH_CTRL_REG);
			writelocal(ai64_device, 0, CONV_COUNTER);	/* reset the counter */
			/* enable the incrementing of the conversion counter */
			writelocal(ai64_device, SC_AIN_ENABLE_SCAN_MASK, 
                                            AI64_SCAN_SYNCH_CTRL_REG);
			not_ai64_board = 0;	/* assume ai64 card */
			{
				#define TEST_RETRY 3
				int retry;
				int new_count, old_count;
				old_count=readlocal(ai64_device, CONV_COUNTER);
				for(retry=0; retry < TEST_RETRY; retry++) {
					msleep_interruptible(1);
					if((new_count=readlocal(ai64_device, CONV_COUNTER)) > 
																old_count) {
						not_ai64_board++;
						if(not_ai64_board == 1) {
			                pci_read_config_dword(pdev, 
                                    (unsigned long) PCI_SUBSYSTEM_VENDOR_ID,
									&subsys_id);
							printk (KERN_INFO AI64_DEVICE_NAME 
								": Device id = %x; Vendor id = %x; "
								"Subsystem id = %x\n", 
								pdev->device, 
								pdev->vendor, 
								subsys_id);

            				printk(KERN_INFO AI64_DEVICE_NAME 
								": /dev/%s%d: PCI Bus# %d, "
                   				"Device# %d.%d, Firmware: 0x%lx\n",
                   				AI64_DEVICE_NAME, ai64_device->minor, 
                                pdev->bus->number, PCI_SLOT(pdev->devfn), 
                                PCI_FUNC(pdev->devfn), (ulong)regval);
						}
						printk(KERN_INFO AI64_DEVICE_NAME
								": %d: ### NOT AI64 BOARD ### : "
								"conversion counter = %d\n",
								not_ai64_board, readlocal(ai64_device, 
															CONV_COUNTER));
					}
					old_count = new_count;
				}
			}

			/* disable the incrementing of the conversion counter */
			writelocal(ai64_device, SC_AIN_RESET_INPUTS_MASK, 
                                            AI64_SCAN_SYNCH_CTRL_REG);
			if (not_ai64_board) {
                /* Unmap the memory space allocated for the device found */
                iounmap(ai64_device->local_reg_address);
                iounmap(ai64_device->config_reg_address);

                /* remove requested memory regions */
                for (i = 0; i < AI64_MAX_REGION; i++) {
                    ai64_deconfigure_mem_regions(ai64_device, i,
                                                 ai64_device->
                                                 mem_region[i].address,
                                                 ai64_device->
                                                 mem_region[i].size,
                                                 ai64_device->
                                                 mem_region[i].flags);
                }
                if(ai64_device->dma)
                    kfree(ai64_device->dma);
                kfree(ai64_device);
                ai64_device=NULL;
                continue;
            }

            init_waitqueue_head(&ai64_device->ioctlwq);
            init_waitqueue_head(&ai64_device->dmawq);
            init_waitqueue_head(&ai64_device->lohiwq);

            ai64_device->pdev = pdev;
            pci_dev_get(ai64_device->pdev);		/* increment reference count */

            ai64_device->irqlevel = pdev->irq;
            ai64_device->busy = 0;
            ai64_device->minor = index;
            ai64_device->next = boards;

            DEBUGPx(D_L1, "base_address[0]=0x%lX, base_address[2]=0x%lX\n",
                    (unsigned long)ai64_device->pdev->resource[0].start,
                    (unsigned long)ai64_device->pdev->resource[2].start);


            AI64_LOCK_INIT();   /* Init the Spin Lock */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
            mutex_init(&ai64_device->ioctl_mtx);
#endif
            REG_DUMP(D_NEVER, ai64_device, "initialize");
            boards = ai64_device;
            index++;

            ai64_device_list[num_boards] = ai64_device;
            ai64_device->bus = pdev->bus->number;   /* pci number */
            ai64_device->slot = PCI_SLOT(pdev->devfn);  /* slot number */
            ai64_device->func = PCI_FUNC(pdev->devfn);  /* function number */
            ai64_device->firmware = (uint)regval; /* firmware revision */

			/* if new AI64SSA/C card, get Master Clock Freq from firmware */
			if(ai64_device->board_type == gsc16ai64ssa_c) {
				masterclk = (ai64_device->firmware >> 17) & 0x3;
				switch(masterclk) {
					case 0:
						ai64_device->MasterClockFreq = 50000000.0;
					break;
					case 1:
						ai64_device->MasterClockFreq = 45000000.0;
					break;
					case 2:
						ai64_device->MasterClockFreq = 49152000.0;
					break;
					case 3:
						ai64_device->MasterClockFreq = 51840000.0;
					break;
					default:
						ai64_device->MasterClockFreq = 50000000.0;
					break;
				}
			} else {
				/* old 16AI64SSA card */
				ai64_device->MasterClockFreq = 30000000.0;
			}

            num_boards++;

            printk(KERN_INFO AI64_DEVICE_NAME ": /dev/%s%d: PCI Bus# %d, "
                   "Device# %d.%d, nbuf=%d, Firmware: 0x%x\n",
                   AI64_DEVICE_NAME, ai64_device->minor, ai64_device->bus,
                   ai64_device->slot, ai64_device->func,
                   ai64_device->num_dma_buffers,
                   ai64_device->firmware);
        }
    }

    if (index == 0) {
        ERRP("No device found\n");
        DEBUGP_EXIT;
        return (-ENODEV);
    }

    device_major = register_chrdev(0, AI64_DEVICE_NAME, &device_fops);
    if (device_major < 0) {
        /* OK, could not register -- undo the work we have done above */
        ERRP("could not register device number\n");
        for (ai64_device = boards; ai64_device; ai64_device = device_next) {
            device_next = ai64_device->next;

            /* Unmap the memory space allocated for the device found */
            iounmap(ai64_device->local_reg_address);
            iounmap(ai64_device->config_reg_address);

            /* remove requested memory regions */
            for (i = 0; i < AI64_MAX_REGION; i++) {
                ai64_deconfigure_mem_regions(ai64_device, i,
                                             ai64_device->mem_region[i].
                                             address,
                                             ai64_device->mem_region[i].
                                             size,
                                             ai64_device->mem_region[i].
                                             flags);
            }
            if(ai64_device->dma)
                kfree(ai64_device->dma);

            pci_dev_put(ai64_device->pdev);	/* decrement reference count */

            kfree(ai64_device);
            ai64_device=NULL;
        }

        boards = NULL;
        DEBUGP_EXIT;
        return (-ENODEV);
    }

    DEBUGPx(D_L1, "Number of devices %d\n", num_boards);
    /*
     *  Add /proc file system support.
     */

    if (num_boards) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
        remove_proc_entry(AI64_DEVICE_NAME, NULL);   /* Just in case. */
#endif

        proc_enabled = 0;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
        {
            struct          proc_dir_entry *proc;
            proc = create_proc_entry(AI64_DEVICE_NAME, S_IRUGO|S_IWUGO, NULL);

            if (proc) {
                proc_enabled = 1;
                //proc->read_proc = read_proc;
                //proc->write_proc = write_proc;
                //proc->get_info = (void *) proc_get_info;
                proc->proc_fops = &ai64_proc_operations;
            } else {
                ERRP("create_proc_entry() failure.\n");
                ai64_exit();
                DEBUGP_EXIT;
                return (-ENOMEM);
            }
        }
#else
        if(!proc_create(AI64_DEVICE_NAME, S_IRUGO|S_IWUGO, NULL, 
                                               &ai64_proc_operations)) {
            ERRP("Could not create /proc entry\n");
            ai64_exit();
            DEBUGP_EXIT;
            return (-ENOMEM);
        }
        proc_enabled = 1;
#endif
    }

    printk(KERN_INFO AI64_DEVICE_NAME
           ": driver version %s successfully inserted.\n", AI64_VERSION);
    DEBUGP_EXIT;
    return 0;
}

/************************************************************************/
/* cleanup when unloading module                                        */
/************************************************************************/
static void ai64_exit(void)
{
    struct ai_board *ai64_device = 0, *device_next;
    int i, regval;

    DEBUGP_ENTER;
    unregister_chrdev(device_major, AI64_DEVICE_NAME);
    for (ai64_device = boards; ai64_device; ai64_device = device_next) {
        /* disable local interrupts */
        regval = readl(IntCntrlStat(ai64_device));
        regval &= (~PCI_INT_ENABLE);
        writel(regval, (IntCntrlStat(ai64_device)));

        /* abort any DMA in progress */
        ai64_abort_any_dma_inprogress(ai64_device);

        /* reset DMA command status register */
        writel((CH0_DMA_CLEAR_IRQ_MASK | CH1_DMA_CLEAR_IRQ_MASK)
               , DMACmdStatus(ai64_device));

        /* reset local interrupt control register */
        writelocal(ai64_device, AI64_ICR_1_IDLE, AI64_INT_CTRL_REG);

        DEBUGPx(D_L1, "device=0x%p\n", ai64_device);
        device_next = ai64_device->next;

        /* Unmap the memory space allocated for the device found */
        iounmap(ai64_device->local_reg_address);
        iounmap(ai64_device->config_reg_address);

        /* remove requested memory regions */
        for (i = 0; i < AI64_MAX_REGION; i++) {
            ai64_deconfigure_mem_regions(ai64_device, i,
                                         ai64_device->mem_region[i].
                                         address,
                                         ai64_device->mem_region[i].size,
                                         ai64_device->mem_region[i].flags);
        }

        if(ai64_device->dma)
            kfree(ai64_device->dma);
			
        pci_dev_put(ai64_device->pdev);	/* decrement reference count */
        
        kfree(ai64_device);
        ai64_device=NULL;
    }
    boards = NULL;
    if (proc_enabled) {
        remove_proc_entry(AI64_DEVICE_NAME, NULL);
        proc_enabled = 0;
    }

    printk(KERN_INFO AI64_DEVICE_NAME
           ": driver version %s successfully removed.\n", AI64_VERSION);
    DEBUGP_EXIT;
}

/************************************************************************/
/* open device                                                          */
/************************************************************************/
static int device_open(struct inode *inode, struct file *fp)
{
    struct ai_board *ai64_device = 0;
    int i, j;

    DEBUGP_ENTER;
    for (ai64_device = boards; ai64_device;
         ai64_device = ai64_device->next) {
        DEBUGPx(D_L2, "ai64_device->minor=%d MINOR(inode->i_rdev)=%d\n",
                ai64_device->minor, MINOR(inode->i_rdev));
        if (MINOR(inode->i_rdev) == ai64_device->minor) {
            if (ai64_device->busy) {
                DEBUGP(D_ERR, "board already opened\n");
                DEBUGP_EXIT;
                return (-EBUSY);
            }

            /* If user has specified DMA buffers */
            if(ai64_device->irqlevel >= 0) {
                if (request_irq
                    (ai64_device->irqlevel, device_interrupt, 
//                     SA_INTERRUPT | SA_SHIRQ, AI64_DEVICE_NAME, 

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
                       IRQF_DISABLED | IRQF_SHARED, AI64_DEVICE_NAME, 
#else
                       IRQF_SHARED, AI64_DEVICE_NAME, 
#endif
                                                         ai64_device) < 0) {
                    DEBUGPx(D_ERR, "can not get interrupt %d\n",
                            ai64_device->irqlevel);
                    ai64_device->busy = 0;
                    DEBUGP_EXIT;
                    return (-EBUSY);
                }
                ai64_device->irq_added = TRUE;
            }

            /*******************************
			 *** use the reserved memory ***
			 *******************************/
            if (ai64_device->use_resmem) {
                for (i = 0; i < ai64_device->num_dma_buffers; i++) {
                    /* virtual memory address */
                    ai64_device->dma[i].dmabufferpool = (u32 *)
                        ioremap(ai64_device->dma[i].dmaoffset, DMA_SIZE);

                    DEBUGPx(D_L2,
                            "Physical[%2d]=0x%lx dmabufferpool=0x%p\n", i,
                            ai64_device->dma[i].dmaoffset,
                            ai64_device->dma[i].dmabufferpool);
                    if (!ai64_device->dma[i].dmabufferpool) {
                        ERRPx("ioremap failed: "
                              "Physical[%2d]=0x%lx dmabufferpool=0x%p\n",
                              i, ai64_device->dma[i].dmaoffset,
                              ai64_device->dma[i].dmabufferpool);
                        for (i = 0; i < ai64_device->num_dma_buffers; i++) {
                            if (ai64_device->dma[i].dmabufferpool)
                                iounmap(ai64_device->dma[i].dmabufferpool);
                            ai64_device->dma[i].dmabufferpool = 0;
                        }
                        if(ai64_device->irq_added == TRUE) {
                            free_irq(ai64_device->irqlevel, ai64_device);
                            ai64_device->irq_added = FALSE;
                        }

                        ai64_device->busy = 0;
                        DEBUGP_EXIT;
                        return (-EIO);
                    }
                    ai64_device->dma[i].bufferstate = DmaFree;
                    ai64_device->dma[i].dmasamples = 0;
                    ai64_device->dma[i].dmastart = 0;
                }

                DEBUGPx(D_L3,
                        "DMA_ORDER=%d DMA_SIZE=0x%p DMA_SAMPLES=%d\n",
                        DMA_ORDER, (void *) DMA_SIZE, DMA_SAMPLES);
            } else {
                /**********************************
				 *** do not use reserved memory ***
				 **********************************/
                // NEW
                // Allocate all DMA buffers
                //
                for (i = 0; i < ai64_device->num_dma_buffers; i++) {

                    DEBUGPx(D_BUFF,
                            "\n*******: ALLOCATE DMA BUFFER MEMORY "
                            "(#%d Buffer), DMA_ORDER (0x%x)  *******\n",
                            i, DMA_ORDER);
                    DEBUGPx(D_BUFF,
                            "*******: DMA_SIZE (0x%x) : DMA_SAMPLES "
                            "(0x%x) PAGE_SIZE (0x%x) *******\n",
                            (unsigned int) DMA_SIZE,
                            (unsigned int) DMA_SAMPLES,
                            (unsigned int) PAGE_SIZE);

                    /* __get_free_pages used to allocated above 16Mb range 
                     * for PCI devices. Using __get_dma_pages will limit 
                     * allocated pages to within 16MB. 
                     */
                    if (!(ai64_device->dma[i].dmabufferpool =
                          (u32 *) __get_free_pages(GFP_KERNEL | GFP_DMA32,
                                                   DMA_ORDER))) {
                        ERRPx("can not allocate DMA pages: "
                              "expected=%d allocated=%d\n",
                              ai64_device->num_dma_buffers, i);

                        ai64_device->busy = 0;
                        DEBUGPx(D_BUFF,
                                "\n\n\n\n*******: RELEASE ALLOCATED MEMORY "
                                "(%d Buffers)  *******\n", i);
                        for(j=0; j < i; j++) {
                            free_pages((uintptr_t) ai64_device->dma[j].
                                       dmabufferpool, DMA_ORDER);
                            ai64_device->dma[j].dmabufferpool = 0;
                        }
                        ai64_device->error = AI64_DEVICE_RESOURCE_ALLOCATION_ERROR;
                        if(ai64_device->irq_added == TRUE) {
                            free_irq(ai64_device->irqlevel, ai64_device);
                            ai64_device->irq_added = FALSE;
                        }
                        DEBUGP_EXIT;
                        return (-ENOMEM);
                    }
                    DEBUGPx(D_BUFF, "### Allocate: %d : 0x%p\n",
                            i, ai64_device->dma[i].dmabufferpool);
                    ai64_device->dma[i].dmaoffset =
                        virt_to_phys((void *) ai64_device->dma[i].
                                     dmabufferpool);

                    /* If DMA address beyond 4GB, error out */
                    if(ai64_device->dma[i].dmaoffset > 0xFFFFFFFFL) {
                        printk (KERN_INFO AI64_DEVICE_NAME 
                                ": memory allocation failed: "
                                "kernel returned memory above 4GB (0x%lx)\n",   
                                ai64_device->dma[i].dmaoffset);

                        for(j=0; j <= i; j++) {
                            free_pages((uintptr_t) ai64_device->dma[j].
                                       dmabufferpool, DMA_ORDER);
                            ai64_device->dma[j].dmabufferpool = 0;
                        }
                        ai64_device->error = AI64_DEVICE_RESOURCE_ALLOCATION_ERROR;
                        if(ai64_device->irq_added == TRUE) {
                            free_irq(ai64_device->irqlevel, ai64_device);
                            ai64_device->irq_added = FALSE;
                        }
                        DEBUGP_EXIT;
                        return (-ENOMEM);
                    }
                    ai64_device->dma[i].bufferstate = DmaFree;
                    ai64_device->dma[i].dmasamples = 0;
                    ai64_device->dma[i].dmastart = 0;
                    DEBUGPx(D_BUFF,
                            "DMA virt=0x%p, phys=0x%lx, status = %d \n",
                            ai64_device->dma[i].dmabufferpool,
                            ai64_device->dma[i].dmaoffset,
                            ai64_device->dma[i].bufferstate);

                }   // End For i
                // End NEW
            }

            // Fill buffer with pattern
            for (i = 0; i < ai64_device->num_dma_buffers; i++) {
                ai64_device->dmabuffer = ai64_device->dma[i].dmabufferpool;
                for (j = 0; j < DMA_SAMPLES; j++) {
                    ai64_device->dmabuffer[j] = 0xDEADBEEF;
                }
            }

#ifdef		DEBUG
            Read_TSC(start_time);
            last_time = start_time;
#endif
            ai64_device->read_in_progress = 0;
            ai64_device->busy = 1;
            ai64_device->dmaState = AI64_DMA_DISABLE;
            ai64_device->dmaSampleSize = DMA_SAMPLES;
            ai64_device->cbuffer_r = 0;
            ai64_device->cbuffer_w = 0;
            ai64_device->buf_inuse_hwm = 0;
            ai64_device->dmainprogress = NO_DMA_INPROGRESS;
            ai64_device->error = AI64_DEVICE_SUCCESS;
            ai64_device->signalno = (-1);
            ai64_device->signalev = NO_EVENT;
            ai64_device->init_pending = FALSE;
            ai64_device->wakeup_dma_pending = FALSE;
            ai64_device->wakeup_lohi_pending = FALSE;
            ai64_device->autocal_pending = FALSE;
            ai64_device->chready_pending = FALSE;
            ai64_device->lo_hi_pending = FALSE;
            ai64_device->hw_init_during_read = 0;
            ai64_device->dma_abort_request = 0;
            ai64_device->read_to_issue_dma = 0;
            ai64_device->timeout_seconds = AI64_DEFAULT_TIMEOUT; //default
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
            init_timer(&ai64_device->watchdog_timer);
            ai64_device->watchdog_timer.function = ai64_timeout_handler;
            ai64_device->watchdog_timer.data = (unsigned long) ai64_device;
#else
            timer_setup(&ai64_device->watchdog_timer, ai64_timeout_handler, 0);
#endif
            ai64_device->bufferThreshold = 0xFFFE;  // ai64_device default
            ai64_device->mmap_reg_select = AI64_SELECT_GSC_MMAP;
            ai64_device->Test_Pattern = 0;  /* for debug only */
            ai64_device->enable_test_pattern = 0;   /* for debug only */
            ai64_device->return_16bit_samples = 0;  /* 32bit samples */
            ai64_device->validate_chan_0 = 0;   /* validate channel 0 */
            ai64_device->bytes_per_sample = sizeof(u32);    /* 32bit samples */
            ai64_device->chan0_index = 0;
            ai64_device->ReadCount = 0;
            ai64_device->WriteCount = 0;
            ai64_device->timestamp_read_hwm = 0;
            ai64_device->timestamp_btw_read_hwm = 0;

#ifdef LINUX_2_4
            init_waitqueue_head(&ai64_device->ioctlwq);
            init_waitqueue_head(&ai64_device->dmawq);
            init_waitqueue_head(&ai64_device->lohiwq);
#else
            ai64_device->ioctlwq = NULL;
            ai64_device->lohiwq = NULL;
#endif
            ai64_device->nchans =
                1 << (readlocal(ai64_device, AI64_SCAN_SYNCH_CTRL_REG) &
                      AI64_SSCR_CHAN_SIZE_MASK);
            fp->private_data = ai64_device;

            ai64_abort_any_dma_inprogress(ai64_device);

            /* reset local IRQ flag */
            DEBUGP(D_L1, "disabling interrupts\n");
            //`         writelocal(ai64_device,0, IntCtrlReg(ai64_device));
            //writel(0, (IntCntrlStat(ai64_device)));
            writel(readl(IntCntrlStat(ai64_device)) | PCI_INT_ENABLE, IntCntrlStat(ai64_device));   //DRD
            if (!try_module_get(THIS_MODULE)) {
                if(ai64_device->irq_added == TRUE) {
                    free_irq(ai64_device->irqlevel, ai64_device);
                    ai64_device->irq_added = FALSE;
                }

                /*** use reserved memory */
                if (ai64_device->use_resmem) {
                    for (i = 0; i < ai64_device->num_dma_buffers; i++) {
                        iounmap(ai64_device->dma[i].dmabufferpool);
                        ai64_device->dma[i].dmabufferpool = 0;
                    }
                } else {
                    /*** do not use reserved memory ***/
                    for (i = 0; i < ai64_device->num_dma_buffers; i++) {
                        if (ai64_device->dma[i].dmabufferpool) {
                            free_pages((uintptr_t) ai64_device->dma[i].
                                       dmabufferpool, DMA_ORDER);
                            DEBUGPx(D_BUFF,
                                    "******* Free Pages %d  0x%p ******\n",
                                    i, ai64_device->dma[i].dmabufferpool);
                            ai64_device->dma[i].dmabufferpool = 0;
                        }
                    }
                }

                ERRP("Unable to increment the module count\n");
                DEBUGP_EXIT;
                return -EBUSY;
            }

            /*** ADD HERE INSTEAD OF DURING EACH DMA ***/
            writel(0, DMAThreshold(ai64_device));   // #1 DMA Threshold 0xB0

            REG_DUMP(D_REGS, ai64_device, "open done");
            DEBUGP(D_L1, "Device open successful\n");
            DEBUGP_EXIT;
            return (0);
        }
    }
    ERRPx("(%d): can not find board\n", MINOR(inode->i_rdev));
    DEBUGP_EXIT;
    return (-ENODEV);
}

/************************************************************************/
/* close device                                                         */
/************************************************************************/
static int device_close(struct inode *inode, struct file *fp)
{
    struct ai_board *ai64_device = (struct ai_board *) fp->private_data;
    unsigned long regval;
    int i;

    DEBUGP_ENTER;

    /* disable local interrupts */
    regval = readl(IntCntrlStat(ai64_device));
    regval &= (~PCI_INT_ENABLE);
    writel(regval, (IntCntrlStat(ai64_device)));

    ai64_device->dma_abort_request++;   /* stop any DMA */

    /* wakeup any pending interrupts */
    if (ai64_device->chready_pending) {
        ai64_device->chready_pending = FALSE;
        ai64_device->wakeup_dma_pending = FALSE;
        wake_up(&ai64_device->dmawq);
    }

    if (ai64_device->lo_hi_pending) {
        ai64_device->lo_hi_pending = FALSE;
        ai64_device->wakeup_lohi_pending = FALSE;
        wake_up(&ai64_device->lohiwq);
    }

    if (ai64_device->dmainprogress) {
        ai64_device->dmainprogress = NO_DMA_INPROGRESS;
        ai64_device->wakeup_dma_pending = FALSE;
        wake_up(&ai64_device->dmawq);
    }

    if (ai64_device->wakeup_dma_pending) {
        ai64_device->wakeup_dma_pending = FALSE;
        wake_up(&ai64_device->dmawq);
    }

    /* let interrupt handler exit gracefully */
    wait_event_interruptible_timeout(ai64_device->ioctlwq,0,
                                   MSECS_TO_SLEEP(20));

    ai64_abort_any_dma_inprogress(ai64_device);

    /* reset DMA command status register */
    writel((CH0_DMA_CLEAR_IRQ_MASK | CH1_DMA_CLEAR_IRQ_MASK)
           , DMACmdStatus(ai64_device));

    /* reset local interrupt control register */
    writelocal(ai64_device, AI64_ICR_1_IDLE, AI64_INT_CTRL_REG);

    /* reset local IRQ flag */
    //` writelocal(ai64_device,0, IntCtrlReg(ai64_device));
    /* free resources */
                        
    if(ai64_device->irq_added == TRUE) {
        free_irq(ai64_device->irqlevel, ai64_device);
        ai64_device->irq_added = FALSE;
    }

    /*** use reserved memory ***/
    if (ai64_device->use_resmem) {
        for (i = 0; i < ai64_device->num_dma_buffers; i++) {
            iounmap(ai64_device->dma[i].dmabufferpool);
            ai64_device->dma[i].dmabufferpool = 0;
        }
    } else {
        /*** do not use reserved memory ***/
        for (i = 0; i < ai64_device->num_dma_buffers; i++) {
            if (ai64_device->dma[i].dmabufferpool) {
                free_pages((uintptr_t) ai64_device->dma[i].dmabufferpool,
                           DMA_ORDER);
                DEBUGPx(D_BUFF, "******* Free Pages %d  0x%p ******\n",
                        i, ai64_device->dma[i].dmabufferpool);
                ai64_device->dma[i].dmabufferpool = 0;
            }
        }
    }

    /* free physical memory if allocated */
    ai64_free_physical_memory(ai64_device);

    ai64_device->busy = 0;
    module_put(THIS_MODULE);    /* decrement count */

    DEBUGP_EXIT;
    return (0);
}

/* write operation: easy -- just return an error */
static ssize_t device_write(struct file *fp, const char *buf, size_t size,
                            loff_t * lt)
{
#ifdef AI64_DEBUG   /* only used for debug message */
    struct ai_board *ai64_device = 0;   /* dummy for use with DEBUGPx*/
#endif                          /* AI64_DEBUG */

    DEBUGP(D_ERR, "Board does not support writes\n");
    return (-EPERM);
}

/************************************************************************/
/* read operation: either polled or uses PLX DMA on CH0                 */
/************************************************************************/
static ssize_t device_read(struct file *fp, char *buf, size_t size,
                           loff_t * lt)
{
    struct ai_board *ai64_device = (struct ai_board *) fp->private_data;
    int nsamples, samples_returned, bytes_read;
    ushort *usp;

    DEBUGP_ENTER;
    /* if no DMA buffers allocated, we cannot perform reads */
    if (ai64_device->num_dma_buffers == 0) {
        DEBUGP(D_ERR, "No DMA buffers specified\n");
        DEBUGP_EXIT;
        return -ENOMEM;
    }

    if (ai64_device->read_in_progress) {
        DEBUGP(D_ERR, "Read in progress\n");
        DEBUGP_EXIT;
        return (-EBUSY);
    }

    if (ai64_device->ioctl_processing) {
        DEBUGP(D_ERR,"ioctl processing active: busy\n");
        DEBUGP_EXIT;
        return (-EBUSY);
    }

    ai64_device->read_in_progress++;

    ai64_device->ReadCount++;
    DEBUGPx(D_NEVER, "READ ENTER - size=0x%p, ReadCount=%d\n",
            (void *) size, ai64_device->ReadCount);
    REG_DUMP(D_NEVER, ai64_device, "Read entered");

    if (ai64_device->ReadCount > 1) {
        TIME_STAMP(D_TIME_BTWR, timestamp_btw_read_end);
        PRINT_TIME(D_TIME_BTWR, "**BTWR** ", timestamp_read_start,
                   timestamp_btw_read_end, timestamp_btw_read_hwm, 0,
                   SKIP_BTWR_BELOW);
    }
    TIME_STAMP((D_RTIME | D_TIME_BTWR), timestamp_read_start);

    if (ai64_device->init_pending) {
        DEBUGP(D_ERR, "Board initialization in progress\n");
        TIME_STAMP(D_RTIME, timestamp_read_end);
        PRINT_TIME(D_RTIME, "**ERR** ", timestamp_read_start,
                   timestamp_read_end, timestamp_read_hwm, 0,
                   SKIP_READ_BELOW);
        DEBUGP_EXIT;
        ai64_device->read_in_progress = 0;
        return (-EBUSY);
    }

    nsamples = size / ai64_device->bytes_per_sample;

    DEBUGPx(D_DMA, "nsamples = %d/0x%p samples/bytes\n",
            nsamples, (void *) size);
    // verify parameters 
    if (nsamples <= 0) {
        DEBUGP(D_ERR, "zero sample return\n");
        TIME_STAMP(D_RTIME, timestamp_read_end);
        PRINT_TIME(D_RTIME, "Zero ", timestamp_read_start,
                   timestamp_read_end, timestamp_read_hwm, 0,
                   SKIP_READ_BELOW);
        DEBUGP_EXIT;
        ai64_device->read_in_progress = 0;
        return (0);
    }

    if (
#ifdef VERIFY_WRITE
            !access_ok(VERIFY_WRITE, (void *) buf, size)
#else
            !access_ok((void *) buf, size)
#endif
            ) {
        DEBUGP(D_ERR, "buffer access not OK\n");
        TIME_STAMP(D_RTIME, timestamp_read_end);
        PRINT_TIME(D_RTIME, "**ERR** ", timestamp_read_start,
                   timestamp_read_end, timestamp_read_hwm, 0,
                   SKIP_READ_BELOW);
        DEBUGP_EXIT;
        ai64_device->read_in_progress = 0;
        return (-EFAULT);
    }

    DEBUGPx(D_L3, "Samples: requested=%d, remain=%d\n", nsamples,
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples);

#if 0
    if ((ai64_device->dmaoverflow == TRUE)
        && (ai64_device->dmaState == AI64_DMA_DISABLE)) {
        // Overflow Cond.
        DEBUGPx(D_ERR,
                "********OVERFLOW Condition Occured******** Return %d\n",
                -ENOBUFS);
        TIME_STAMP(D_RTIME, timestamp_read_end);
        PRINT_TIME(D_RTIME, "**ERR** ", timestamp_read_start,
                   timestamp_read_end, timestamp_read_hwm, 0,
                   SKIP_READ_BELOW);
        DEBUGP_EXIT;
        ai64_device->read_in_progress = 0;
        return (-ENOBUFS);
    }
#endif
    //
    // Continuous DMA Mode I/O - Read from circular buffer list
    //

    ai64_device->hw_init_during_read = ai64_device->dma_abort_request = 0;

    if (ai64_device->dmaState == AI64_DMA_CONTINUOUS) {
                                                // Read direct from DMA buffers
        samples_returned = device_read_continuous(ai64_device, buf, nsamples,
                                   (fp->f_flags & O_NONBLOCK));
        DEBUGPx(D_L3,
                "Samples requested: %d , Samples returned: %d\n",
                nsamples, samples_returned);
        DEBUGP_EXIT;

        if (samples_returned < 0) {
            TIME_STAMP(D_RTIME, timestamp_read_end);
            PRINT_TIME(D_RTIME, "Zero ", timestamp_read_start,
                       timestamp_read_end, timestamp_read_hwm, 0,
                       SKIP_READ_BELOW);
            DEBUGP_EXIT;
            ai64_device->read_in_progress = 0;
            return (samples_returned);
        }

        bytes_read = samples_returned * ai64_device->bytes_per_sample;
        TIME_STAMP(D_RTIME, timestamp_read_end);
        PRINT_TIME(D_RTIME, "**OK**: ", timestamp_read_start,
                   timestamp_read_end, timestamp_read_hwm, bytes_read,
                   SKIP_READ_BELOW);

        DEBUGPx(D_NEVER, "READ EXIT - bytes_read=%d\n", bytes_read);
        DEBUGP_EXIT;
        ai64_device->read_in_progress = 0;
        return (bytes_read);
    }
    //
    //
    // Not Continous DMA Mode - Use only the first buffer over and over again. 
    //    Best Results when nsamples == threshold value!!
    //
    //

    ai64_device->dmabuffer =
        ai64_device->dma[ai64_device->cbuffer_r].dmabufferpool;

    DEBUGPx(D_DMA, "Set Current Buffer: ai64_device->dmabuffer = 0x%p, "
            "ai64_device->dma[%d].dmabufferpool = 0x%p\n",
            ai64_device->dmabuffer, ai64_device->cbuffer_r,
            ai64_device->dma[ai64_device->cbuffer_r].dmabufferpool);


    DEBUGPx(D_L3, "local copy dmastart (%d) for %d dmasamples\n",
            ai64_device->dma[ai64_device->cbuffer_r].dmastart,
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples);
    // check whether we have anything left in the buffer.  If so, return that.
    if (ai64_device->dma[ai64_device->cbuffer_r].dmasamples > 0) {


        DEBUGPx(D_L3, "First Sample Value to Return: dmabuffer[%d]@0x%p: "
                "0x%x\n",
                ai64_device->dma[ai64_device->cbuffer_r].dmastart,
                &ai64_device->dmabuffer[ai64_device->
                                        dma[ai64_device->cbuffer_r].
                                        dmastart],
                ai64_device->dmabuffer[ai64_device->
                                       dma[ai64_device->cbuffer_r].
                                       dmastart]);
        DEBUGPx(D_L1,
                "Local data copy......REQUEST nsamples(%d)/ "
                "HAVE dmasamples(%d)\n", nsamples,
                ai64_device->dma[ai64_device->cbuffer_r].dmasamples);

        // Check to see if more samples are requested than remaining in buffer
        if (nsamples > ai64_device->dma[ai64_device->cbuffer_r].dmasamples) {
            nsamples = ai64_device->dma[ai64_device->cbuffer_r].dmasamples;
            // Return only what is left in the buffer
            if (ai64_device->return_16bit_samples) {
                usp = (ushort *) ai64_device->dmabuffer;
                if(__copy_to_user(buf,
                               &usp[ai64_device->
                                    dma[ai64_device->cbuffer_r].dmastart],
                               nsamples * ai64_device->bytes_per_sample)) {
                    ai64_device->read_in_progress = 0;
                    DEBUGP_EXIT;
                    return (-EFAULT);
                }
            } else {
                if(__copy_to_user(buf,
                               &ai64_device->dmabuffer[ai64_device->
                                    dma[ai64_device->cbuffer_r].dmastart],
                               nsamples * ai64_device->bytes_per_sample)) {
                    ai64_device->read_in_progress = 0;
                    DEBUGP_EXIT;
                    return (-EFAULT);
                }
            }
            ai64_device->dma[ai64_device->cbuffer_r].dmastart = 0;
            // Leave Pointer at the beginning
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples = 0;
            // Indicate 0 samples left

            DEBUGP(D_L1, "******DMA BUFFER NOW EMPTY*******\n");

        } else {
            if (ai64_device->return_16bit_samples) {
                usp = (ushort *) ai64_device->dmabuffer;
                if(__copy_to_user(buf,
                               &usp[ai64_device->
                                    dma[ai64_device->cbuffer_r].dmastart],
                               nsamples * ai64_device->bytes_per_sample)) {
                    ai64_device->read_in_progress = 0;
                    DEBUGP_EXIT;
                    return (-EFAULT);
                }
            } else {
                if(__copy_to_user(buf,
                               &ai64_device->dmabuffer[ai64_device->
                                                       dma[ai64_device->
                                                           cbuffer_r].
                                                       dmastart],
                               nsamples * ai64_device->bytes_per_sample)) {
                    ai64_device->read_in_progress = 0;
                    DEBUGP_EXIT;
                    return (-EFAULT);
                }
            }
            ai64_device->dma[ai64_device->cbuffer_r].dmastart += nsamples;
            // Leave pointing to next sample in buffer
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples -=
                nsamples;
            // Indicate how many samples remain.

            DEBUGPx(D_L1,
                    "DMA BUFFER %d Samples Remaining @ %d offset\n",
                    ai64_device->dma[ai64_device->cbuffer_r].dmasamples,
                    ai64_device->dma[ai64_device->cbuffer_r].dmastart);
        }

        DEBUGPx(D_L1,
                "after local data copy... return(%d samples/%d bytes)\n",
                nsamples, (int) (nsamples * sizeof(u32)));

        bytes_read = nsamples * ai64_device->bytes_per_sample;
        TIME_STAMP(D_RTIME, timestamp_read_end);
        PRINT_TIME(D_RTIME, "**OK**: ", timestamp_read_start,
                   timestamp_read_end, timestamp_read_hwm, bytes_read,
                   SKIP_READ_BELOW);
        DEBUGP_EXIT;
        ai64_device->read_in_progress = 0;
        return (bytes_read);
    }

    DEBUGPx(D_L4,
            "Read at most one buffer full: DMA_SAMPLES = %d / 0x%x\n",
            (unsigned int) DMA_SAMPLES, (unsigned int) DMA_SAMPLES);
    DEBUGPx(D_L4,
            "Read at most one buffer full: DMA_ORDER = %d / 0x%x\n",
            (unsigned int) DMA_ORDER, (unsigned int) DMA_ORDER);
    DEBUGPx(D_L4,
            "Read at most one buffer full: DMA_SIZE = %d / 0x%x\n",
            (unsigned int) DMA_SIZE, (unsigned int) DMA_SIZE);

    /* read at most a dmabuffer full at a time (0x10000 samples) */
    if (nsamples > DMA_SAMPLES) {
        DEBUGPx(D_L1,
                "nsamples too large: nsamples(%d) > DMA_SAMPLES(%d) \n",
                (unsigned int) nsamples, (unsigned int) DMA_SAMPLES);
        nsamples = DMA_SAMPLES;
    }

    DEBUGP(D_L1, "  DMA BUFFER EMPTY - Replenish from onboard FIFO\n");

    /* fill the DMA buffer and satisfy the user request from it */
    samples_returned =
        device_read_buffer(ai64_device, buf, nsamples,
                           (fp->f_flags & O_NONBLOCK));
    DEBUGPx(D_NEVER, "samples_returned = %d\n", samples_returned);

    bytes_read = nsamples * ai64_device->bytes_per_sample;
    TIME_STAMP(D_RTIME, timestamp_read_end);
    PRINT_TIME(D_RTIME, "**OK**: ", timestamp_read_start,
               timestamp_read_end, timestamp_read_hwm, bytes_read,
               SKIP_READ_BELOW);
    DEBUGP_EXIT;
    ai64_device->read_in_progress = 0;
    return (samples_returned);

}   // device_read()

/************************************************************************/
/*  device_read_buffer                                                  */
/*                                                                      */
/* Buffered I/O read method (Non-Continuous DMA)                        */
/* The method first attempts to fill the DMA buffer to a "reasonable"   */
/* level then satisfies the user request from it. Any leftover data     */
/* will be returned by successive read call(s)                          */
/************************************************************************/
static int device_read_buffer(struct ai_board *ai64_device, char *buf,
                              int nsamples, int noblock)
{
    int fifosamples, i;
    int retcode;

    DEBUGP_ENTER;

    switch (ai64_device->dmaState) {
        // for PIO and regular DMA, wait for sufficient samples
    case AI64_DMA_DISABLE:  // PIO
    case AI64_DMA_ENABLE:
    //case AI64_DMA_DEMAND_MODE:	/* DRD 01/07/2014 - Demand DMA fix */

        AI64_LOCK();     /** Set the lock **/

        // check how much data there is in the FIFO 
        fifosamples = readlocal(ai64_device, AI64_BUFFER_SIZE_REG);

        DEBUGPx(D_L3, "FIFO ###AI64_DMA_ENABLE/PIO/DEMAND_DMA ### - "
                "FIFO Size %d Samples Request: %d Samples\n",
                fifosamples, nsamples);

        if (fifosamples < nsamples) // wait for more data in FIFO 
        {
            // first of all, check for non-blocking I/O.
            if (noblock) {

                AI64_UNLOCK();     /** Free the lock **/

                DEBUGP(D_ERR, "return warning/error noblock\n");

                ai64_device->error = AI64_DEVICE_SUCCESS;

                DEBUGP_EXIT;
                return (-EAGAIN);
            }

            DEBUGP(D_L4,
                    "DMA_MODE/PIO MODE - waiting for more data...\n");

            writelocal(ai64_device, AI64_ICR_1_LO_HI, AI64_INT_CTRL_REG);

            ai64_device->timeout = FALSE;

            ai64_device->lo_hi_pending = TRUE;
            ai64_device->wakeup_lohi_pending = TRUE;

            ai64_device->watchdog_timer.expires =
                jiffies + ai64_device->timeout_seconds * HZ;
            add_timer(&ai64_device->watchdog_timer);

            // writel(readl(IntCntrlStat(ai64_device)) | IRQ_PCI_ENABLE | 
            //               IRQ_LOCAL_PCI_ENABLE, IntCntrlStat(ai64_device)); 
            writel(readl(IntCntrlStat(ai64_device)) | PCI_INT_ENABLE, IntCntrlStat(ai64_device));   //DRD

            AI64_UNLOCK();     /** Free the lock **/

            DEBUGPx(D_WAIT,
                    "Wait for interrupt: lohiwq: lo_hi_pending=%d\n",
                    ai64_device->lo_hi_pending);

            /* if number of samples in fifo has exceeded nsamples, then we will not
             * get an interrupt. In that case, skip waiting for interrupt.
             */
            fifosamples = readlocal(ai64_device, AI64_BUFFER_SIZE_REG);

            if(fifosamples < nsamples) {
                wait_event_interruptible(ai64_device->lohiwq,
                                     (!ai64_device->lo_hi_pending));

                DEBUGPx(D_WAIT, "Wait over: lohiwq: lo_hi_pending=%d\n",
                        ai64_device->lo_hi_pending);
    
                if (ai64_device->timeout) {
                    DEBUGP(D_ERR,
                            "device_read_buffer - timeout waiting for FIFO ...\n");
    
                    DEBUGPx(D_PLX,
                            "In progress: %d PLX int/ctrl %.8X BC %.8X DMA %.8X\n",
                            ai64_device->dmainprogress,
                            readl(IntCntrlStat(ai64_device)),
                            readlocal(ai64_device, AI64_BOARD_CTRL_REG),
                            readl(DMACmdStatus(ai64_device)));
    
    
                    ai64_device->error = AI64_DEVICE_PIO_TIMEOUT;
                    DEBUGP_EXIT;
                    return (-ETIME);
                } else
                    del_timer_sync(&ai64_device->watchdog_timer);
    
                if (signal_pending(current)) {
                    DEBUGP(D_ERR, "signal pending\n");
                    if (ai64_device->timeout == FALSE)
                        del_timer_sync(&ai64_device->watchdog_timer);
                    DEBUGP_EXIT;
                    return (-EINTR);
                }
    
                fifosamples = readlocal(ai64_device, AI64_BUFFER_SIZE_REG);
            } else {
                if(ai64_device->timeout == FALSE) /* DRD-BUG 01/07/2014: If timeout has not occured, delete it */
				    del_timer_sync(&ai64_device->watchdog_timer);
            }

            // update onboard samples available

            // if still no data present, something is wrong.
            if (fifosamples <= 0) {
                DEBUGP(D_ERR, "no data error\n");
                DEBUGP_EXIT;
                return (-EIO);
            }
            // Overflow of FIFO?
            if (fifosamples >= (MAX_DMA_SAMPLES - 1)) {
                DEBUGPx(D_ERR, "*****FIFO OVERFLOW*******: fifosamples=%d,DMA_SAMPLES=%d\n",
                        fifosamples, DMA_SAMPLES);
                DEBUGP_EXIT;
                return (-ENOBUFS);
            }
        } else {

            // Overflow of FIFO?
            if (fifosamples >= (MAX_DMA_SAMPLES - 1)) {
                if (ai64_device->dmaState == AI64_DMA_ENABLE) {
                    DEBUGP(D_ERR,
                            "*****DMA ENABLE MODE FIFO OVERFLOW*******\n");
                } else {
                    DEBUGP(D_ERR,
                            "*****DMA DISABLE (PIO) MODE FIFO OVERFLOW*******\n");
                }

                AI64_UNLOCK();     /** Free the lock **/
                DEBUGP_EXIT;
                return (-ENOBUFS);
            }
                
            writel(readl(IntCntrlStat(ai64_device)) | PCI_INT_ENABLE,
                       IntCntrlStat(ai64_device));

            AI64_UNLOCK();     /** Free the lock **/
        }
        break;

#if 1	/* DRD 01/07/2014 - Demand DMA fix */
		case AI64_DMA_DEMAND_MODE:
        	AI64_LOCK();     /** Set the lock **/
            fifosamples = readlocal(ai64_device, AI64_BUFFER_SIZE_REG);
			DEBUGPx(D_L3,"fifosamples=0x%08x nsamples=0x%08x\n",fifosamples,nsamples);

            // Overflow of FIFO?
            if (fifosamples >= (MAX_DMA_SAMPLES - 1)) {
                if (ai64_device->dmaState == AI64_DMA_ENABLE) {
                    DEBUGP(D_ERR,
                            "*****DMA ENABLE MODE FIFO OVERFLOW*******\n");
                } else {
                    DEBUGP(D_ERR,
                            "*****DMA DISABLE (PIO) MODE FIFO OVERFLOW*******\n");
                }

                AI64_UNLOCK();     /** Free the lock **/
                DEBUGP_EXIT;
                return (-ENOBUFS);
            }
            writel(readl(IntCntrlStat(ai64_device)) | PCI_INT_ENABLE,
                       IntCntrlStat(ai64_device));
            AI64_UNLOCK();     /** Free the lock **/
        break;
#endif

#ifdef GS_DEMAND_DMA_MODE_AS_DOCUMENTED
        /*                                                               *
         * The Demand Mode DMA does not actually function as documented. *
         * The code below assumes you can start a DMA before the FIFO    *
         * has reached the DMA xfer count it will continue the DMA until *
         * the xfer count has been reached.  Reality is that the DMA     *
         * halts as soon as the FIFO is empty.                           *
         */


        // for demand mode, wait for the DMA done interrupt.
    case AI64_DMA_DEMAND_MODE:
        //  memset(ai64_device->dmabuffer,0, DMA_ORDER*PAGE_SIZE);
        //  {
        //      int i;
        //      for(i=0;i<10;i++)
        //      {
        //          DEBUGPx(D_L3,"%lX\n",
        //              ai64_device->dmabuffer[i]);
        //      }
        //      i=0;
        //  }

        //
        // check how much data there is in the FIFO 
        //
        fifosamples = readlocal(ai64_device, AI64_BUFFER_SIZE_REG);
        // Overflow of FIFO?
        if (fifosamples >= (MAX_DMA_SAMPLES - 1)) {
            DEBUGP(D_ERR, "*****DEMAND MODE FIFO OVERFLOW*******\n");
            DEBUGP_EXIT;
            return (-ENOBUFS);
        }

        DEBUGPx(D_L3, "FIFO ###AI64_DMA_DEMAND### - FIFO Samples %d\n",
                fifosamples);
        DEBUGPx(D_L3, "REQUEST: Samples %d\n", nsamples);


        //
        // Perform DEMAND_MODE DMA Transfer
        //
        if (device_read_dma(ai64_device, nsamples, AI64_DMA_DEMAND_MODE) != 0) {
            DEBUGP(D_ERR,
                    "device_read_buffer - demand mode device_read_dma failure\n");
            DEBUGP_EXIT;
            return (-EAGAIN);
        } else {
            DEBUGP(D_L3, "device_read_buffer - demand mode success\n");
            fifosamples = nsamples; // actually samples to copy.
        };
        break;
#endif                          /* GS DEMAND_MODE_AS DOCUMENTED */

    default:   // invalid state
        DEBUGP(D_ERR, "device_read_buffer - invalid state failure\n");
        DEBUGP_EXIT;
        return (-EIO);
        break;
    }   // switch

// move the data from the hardware buffer to the intermediate buffer by the 
// user's chosen method.

    switch (ai64_device->dmaState) {
    case AI64_DMA_DISABLE:  // PIO

        DEBUGPx(D_L3, "PIO starting addr 0x%p nsamples=0x%X fifosamples=0x%X\n",
                &ai64_device->dmabuffer[0], nsamples, fifosamples);

        for (i = 0; i < nsamples; i++) {
            ai64_device->dmabuffer[i] =
                readlocal(ai64_device, AI64_INPUT_DATA_BUFF_REG);
        }
        ai64_device->dma[ai64_device->cbuffer_r].dmasamples = nsamples;
        // Keep track of how many samples in buffer (RAC)
        /* now check for buffer validation and 16/32 bit xfers */

        if((retcode = ai64_validate_data(ai64_device))) {
            return(retcode);
        }

        DEBUGPx(D_L3, "PIO after addr 0x%p nsamples=0x%X fifosamples=0x%X\n",
                &ai64_device->dmabuffer[0], nsamples, fifosamples);

        break;

    case AI64_DMA_ENABLE:
    case AI64_DMA_DEMAND_MODE:

        DEBUGPx(D_L2,
                "DMA %d nsamples into local buffer: dmastart(%d), "
                "dmasamples(%d) data=0x%x\n", nsamples,
                ai64_device->dma[ai64_device->cbuffer_r].dmastart,
                ai64_device->dma[ai64_device->cbuffer_r].dmasamples,
                (unsigned int) ai64_device->dmabuffer[ai64_device->
                                                      dma[ai64_device->
                                                          cbuffer_r].
                                                      dmastart]);

        // Go DMA all available data samples in FIFO
        if ((retcode=device_read_dma(ai64_device, nsamples, 
                                ai64_device->dmaState)) != 0) {
            DEBUGP(D_ERR,
                    "device_read_buffer - device_read_dma failure\n");
            DEBUGP_EXIT;
            return (retcode);
        };

        /** THERE APPEARS TO BE A SPURIOUS INTERRUPT BEING GENERATED AFTER 
         ** A MASTER ABORT HAS BEEN ISSUED AND THE FIRST DMA IS REQUESTED.
         ** THIS IS USUALLY BECAUSE THE MASTER ABORT WAS ISSUED WHEN NO DMA
         ** WAS PENDING. TO IGNORE THIS SPURIOUS INTERRUPT, SIMPLY SET A FLAG
         ** 'DEADBEEF' IN THE FIRST SAMPLE OF THE BUFFER AND VERIFY THAT THE 
         ** SAMPLE WAS NOT UPDATED. IN THAT CASE, RETURN ZERO BYTES TO THE USER.
         **/
        if(ai64_device->dmabuffer[0] == 0xDEADBEEF) {
            DEBUGPx(D_ERR, "EMPTY DMA READ - buffer number %d!!!\n",
                    ai64_device->cbuffer_r);
            DEBUGP_EXIT;
            return (0); /* return zero bytes */
        }

        ai64_device->dma[ai64_device->cbuffer_r].dmasamples = nsamples;

        /* now check for buffer validation and 16/32 bit xfers */
        if((retcode = ai64_validate_data(ai64_device))) {
            return(retcode);
        }
        // Keep track of how many samples in buffer (RAC)
        break;

#ifdef GS_DEMAND_DMA_MODE_AS_DOCUMENTED
    case AI64_DMA_DEMAND_MODE:
        // already in the intermediate buffer.
        break;
#endif

    default:   // invalid state
        DEBUGP_EXIT;
        DEBUGPx(D_ERR, "bad dmaState=%d\n", ai64_device->dmaState);
        return (-EIO);
        break;
    }

// Now the data is in the intermediate buffer.  Copy it to the user 
// buffer and return.

    DEBUGPx(D_L3, "after xfer: Current Samples in FIFO = %d\n",
            readlocal(ai64_device, AI64_BUFFER_SIZE_REG));
    DEBUGPx(D_L3, "nsamples = 0x%x (%d), dmasamples = 0x%x (%d)\n", nsamples,nsamples,
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples,
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples);

    DEBUGPx(D_L3, "Sample begining value dmabuffer[%d] @ 0x%p: 0x%x\n",
            ai64_device->dma[ai64_device->cbuffer_r].dmastart,
            &ai64_device->dmabuffer[0],
            (unsigned int) ai64_device->dmabuffer[0]);

    DEBUGPx(D_L3, "Sample first value dmabuffer[%d] @ 0x%p: 0x%x\n",
            ai64_device->dma[ai64_device->cbuffer_r].dmastart,
            &ai64_device->dmabuffer[ai64_device->
                           dma[ai64_device->cbuffer_r].dmastart],
            (unsigned int) ai64_device->dmabuffer[ai64_device->
                                    dma[ai64_device->cbuffer_r].dmastart]);
    DEBUGPx(D_L3, "Sample previous last value in "
            "dmabuffer[%d] @ 0x%p: 0x%x\n",
            ai64_device->dma[ai64_device->cbuffer_r].dmastart + nsamples - 1,
            &ai64_device->dmabuffer[ai64_device->
                    dma[ai64_device->cbuffer_r].dmastart + nsamples - 1],
            (unsigned int) ai64_device->dmabuffer[ai64_device->
                    dma[ai64_device->cbuffer_r].dmastart + nsamples - 1]);
    //
    // Partial Read of dmabuffer - Copy needed data from dmabuffer
    //
    if(__copy_to_user(buf, ai64_device->dmabuffer, nsamples *
                   ai64_device->bytes_per_sample)) {
        DEBUGP_EXIT;
        return (-EFAULT);
    }

    ai64_device->dma[ai64_device->cbuffer_r].dmasamples -= nsamples;
    // The remaining samples are still avail.
    if(ai64_device->dma[ai64_device->cbuffer_r].dmasamples == 0)
        ai64_device->dma[ai64_device->cbuffer_r].dmastart = 0;
    else
        ai64_device->dma[ai64_device->cbuffer_r].dmastart = nsamples;

    DEBUGPx(D_L3,
            "DMA Buffer state after copy to userbuf - %d (dmasamples) "
            "starting at %d (dmastart)\n",
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples,
            ai64_device->dma[ai64_device->cbuffer_r].dmastart);

    DEBUGP_EXIT;

    return (nsamples * sizeof(u32));

}   // device_read_buffer

/***********************************************************************************************/
// device_read_dma()
/***********************************************************************************************/

/* fill the buffer using the DMA method */
/* by the time this is called, we know that we have enough samples */
/* in the FIFO */
static int device_read_dma(struct ai_board *ai64_device, int nsamples,
                           int DMAMode)
{
    unsigned long regval;

    DEBUGP_ENTER;

    /* sanity check - should never happen */
    if (ai64_device->dmainprogress == DMA_INPROGRESS) {
        DEBUGP(D_ERR, "##### DMA ALREADY IN PROGRESS #####\n");
        DEBUGP_EXIT;
        return (-EAGAIN);
    }

    AI64_LOCK(); /** Set the lock **/

    ai64_device->dmainprogress = DMA_INPROGRESS;

    /*** This is a klooge to check for empty DMA read ***/
    ai64_device->dmabuffer[0] = 0xDEADBEEF;

    writel(0, DMAThreshold(ai64_device));   // #1

    if (DMAMode == AI64_DMA_ENABLE) {
        writel(NON_DEMAND_DMA_MODE, DMAMode0(ai64_device)); // #2
    } else {
        writel(DEMAND_DMA_MODE, DMAMode0(ai64_device)); // #2
    }

    DEBUGPx(D_DMA,
            "DMA Physical Address dmaoffset[0] =  0x%08lx \n",
            ai64_device->dma[ai64_device->cbuffer_r].dmaoffset);

    writel(ai64_device->dma[ai64_device->cbuffer_r].dmaoffset, DMAPCIAddr0(ai64_device));   // #3
    writel((AI64_INPUT_DATA_BUFF_REG << 2), DMALocalAddr0(ai64_device)); // #4
    writel((nsamples << 2), DMAByteCnt0(ai64_device));  // #5

    // writel(0xA, DMADescrPtr0(ai64_device));
    writel(0xC, DMADescrPtr0(ai64_device)); // #6
    //
    //writel(0, DMAArbitr(ai64_device));
    writel((1 << 19), DMAArbitr(ai64_device));  // #7 MARB Mode/Arbitration (DMA0 Pri) 
    // 0xAC   **GT**
    regval = readl(DMACmdStatus(ai64_device));
    regval &= STOP_DMA_CMD_0_MASK;
    writel(regval, DMACmdStatus(ai64_device));  // #8

    // OK, here we go! - 
    regval = readl(DMACmdStatus(ai64_device));
    regval |= SETUP_DMA_CMD_0;
    writel(regval, DMACmdStatus(ai64_device));  // #9

    DEBUGP(D_DMA, "device_read_dma - about to sleep...\n");

#if 0	/* DRD-BUG 01/07/2014: Don't set timer here */
    ai64_device->timeout = FALSE;
    ai64_device->watchdog_timer.expires =
        jiffies + ai64_device->timeout_seconds * HZ;
    add_timer(&ai64_device->watchdog_timer);
#endif

    writel(readl(IntCntrlStat(ai64_device)) | IRQ_PCI_ENABLE |
           IRQ_LOCAL_PCI_ENABLE, IntCntrlStat(ai64_device));

    /*** Check for FIFO overflow ***/
    ai64_device->dma[ai64_device->cbuffer_r].fifo_read =
        readlocal(ai64_device, AI64_BUFFER_SIZE_REG);

    /*** check for overflow of fifo ***/
    if (ai64_device->dma[ai64_device->cbuffer_r].fifo_read >=
        (MAX_DMA_SAMPLES - ai64_device->nchans)) {
        DEBUGPx(D_ERR, "- AI64_DMA_ENABLE/DEMAND MODE: FIFO overflow - "
                "read too slow (fifo=0x%x)\n",
                ai64_device->dma[ai64_device->cbuffer_w].fifo_read);
        ai64_device->error = AI64_DEVICE_FIFO_OVERFLOW;
        ai64_device->dma_abort_request++;
        ai64_device->dma[ai64_device->cbuffer_r].bufferstate = DmaOverflow;
        regval = readl(DMACmdStatus(ai64_device));  // DMA CSR 0xA8
        regval &= ~STARTUP_DMA_CMD_0;
        writel(regval, DMACmdStatus(ai64_device));
    }

    /* if hardware has been initialized, skip starting DMA */
    if (ai64_device->hw_init_during_read || ai64_device->dma_abort_request) {
        ai64_device->dmainprogress = NO_DMA_INPROGRESS;
        DEBUGP(D_ERR,
                "HW Initialized or DMA abort request prior to starting DMA\n");
        DEBUGP_EXIT;
        AI64_UNLOCK(); /** Free the lock **/
        if(ai64_device->dma_abort_request)
            return (-ENOBUFS);
        else
            return (-EINTR);
    }

#if 1	/* DRD-BUG 01/07/2014: set timer here */
    ai64_device->timeout = FALSE;
    ai64_device->watchdog_timer.expires =
        jiffies + ai64_device->timeout_seconds * HZ;
    add_timer(&ai64_device->watchdog_timer);
#endif

    DEBUGP(D_DMA, "########## DMA BEING STARTED ##########\n");
    // Start the DMA
    writel((regval | STARTUP_DMA_CMD_0), DMACmdStatus(ai64_device)); 

    AI64_UNLOCK(); /** Free the lock **/

    DEBUGPx(D_WAIT, "Wait for interrupt: dmawq: dmainprogress=%d\n",
            ai64_device->lo_hi_pending);
    wait_event_interruptible(ai64_device->dmawq,
                             (!ai64_device->dmainprogress));
    //writel(0xffffffff, (Mailbox0(ai64_device)));
    DEBUGPx(D_WAIT, "Wait over: dmawq: dmainprogress=%d\n",
            ai64_device->lo_hi_pending);

    if (ai64_device->timeout) {
        DEBUGP(D_ERR, "device_read_dma - timeout...\n");
        DEBUGPx(D_L3,
                "DMA In progress: %d PLX int/ctrl %.8X BC %.8X DMA %.8X\n",
                ai64_device->dmainprogress,
                readl(IntCntrlStat(ai64_device)), readlocal(ai64_device,
                                                      AI64_BOARD_CTRL_REG),
                readl(DMACmdStatus(ai64_device)));
        DEBUGP_EXIT;
        return (-EAGAIN);
    } else {
        DEBUGP(D_DMA, "device_read_dma - success...\n");
        del_timer_sync(&ai64_device->watchdog_timer);
    }

    DEBUGP_EXIT;
    return 0;
}

/****************************************************************************/
/* Continuous DMA Mode - Read directly from DMA Buffers                     */
/****************************************************************************/
static int device_read_continuous(struct ai_board *ai64_device, char *buf,
                                  int nsamples, int noblock)
{
    int samples_to_return, mode;
    int retcode;
    ushort *usp;

    AI64_LOCK(); /** Set the lock **/
    DEBUGP_ENTER;

    DEBUGPx(D_L2, "### CMDS=0x%x ICR=0x%x ICSR=0x%x\n",
            readl(DMACmdStatus(ai64_device)),
            readl(IntCtrlReg(ai64_device)),
            readl(IntCntrlStat(ai64_device)));
    DEBUGPx(D_L2, "### BCR=0x%x SSCR=0x%x\n",
            readl(BoardCtrlReg(ai64_device)),
            readl(ScanSynchCtrlReg(ai64_device)));
    DEBUGPx(D_L2, "### GEN: A=%8.8X B=%8.8X\n",
            readl(RateAGenReg(ai64_device)),
            readl(RateBGenReg(ai64_device)));

    // check how much data there is in the FIFO 
    DEBUGPx(D_L4, "fifosamples=%d MAX_DMA_SAMPLES=%d\n",
            readlocal(ai64_device, AI64_BUFFER_SIZE_REG), MAX_DMA_SAMPLES);

    if (ai64_device->dma[ai64_device->cbuffer_r].bufferstate == DmaFree) {

        /* if no dma in progress */
        if (!(ai64_device->dmainprogress == DMA_INPROGRESS)) {
			{
				/* DMA would not start after a timeout */
    			unsigned long regval;
    			regval = readl(DMACmdStatus(ai64_device));
    			regval |= SETUP_DMA_CMD_0;
    			writel(regval, DMACmdStatus(ai64_device));  // #9
			}

            /* if the AI64_ICR_1_LO_HI is not set (usually the first read) and
             * we have exceeded the fifo to generate an interrupt, issue
             * the DMA on behalf of the interrupt handler.
             */

            if (!(readlocal(ai64_device, AI64_INT_CTRL_REG) & AI64_ICR_1_LO_HI)) {
                writelocal(ai64_device, AI64_ICR_1_LO_HI, AI64_INT_CTRL_REG);
                writel(readl(IntCntrlStat(ai64_device)) | PCI_INT_ENABLE,
                       IntCntrlStat(ai64_device));
#if 0 /* DRD-BUG 01/10/2014: We were timing out for small sample size (700) */
                if (readlocal(ai64_device, AI64_BUFFER_SIZE_REG) >=
                    ai64_device->bufferThreshold) {
                    DEBUGP(D_DMA, "#### READ ISSUING DMA (A) ####\n");
                    ai64_fire_dma(ai64_device);
                }
#endif
            }
#if 1 /* DRD-BUG 01/10/2014: Fire DMA if we have enough samples */
            if (readlocal(ai64_device, AI64_BUFFER_SIZE_REG) >=
                ai64_device->bufferThreshold) {
                DEBUGP(D_DMA, "#### READ ISSUING DMA (A) ####\n");
                ai64_fire_dma(ai64_device);
            }
#endif
        }

        /*** Check for FIFO overflow ***/
        if (readlocal(ai64_device, AI64_BUFFER_SIZE_REG) >=
            (MAX_DMA_SAMPLES - 1)) {
            DEBUGP(D_ERR,
                    " - AI64_DMA_CONTINUOUS MODE: FIFO overflow - read too slow\n");
            ai64_device->error = AI64_DEVICE_FIFO_OVERFLOW;
            ai64_abort_any_dma_inprogress(ai64_device);
            DEBUGP_EXIT;
            AI64_UNLOCK(); /** Free the lock **/
            return (-ENOBUFS);	/* DRD-BUG 1/7/2013 - generate alternate error */
        }

        //
        // first of all, check for non-blocking I/O.
        //

        if (noblock) {
            DEBUGP(D_ERR,
                    " - AI64_DMA_CONTINUOUS MODE: return warning/error noblock\n");
            ai64_device->error = AI64_DEVICE_SUCCESS;
            DEBUGP_EXIT;
            AI64_UNLOCK(); /** Free the lock **/
            return (-EAGAIN);
        }

        ai64_device->timeout = FALSE;
        // ai64_device->wakeup_dma_pending = TRUE; /* 01/10/2014: move it prior to wait */
        // Let Continuous Mode DMA know we are waiting
        ai64_device->watchdog_timer.expires =
            jiffies + ai64_device->timeout_seconds * HZ;
        add_timer(&ai64_device->watchdog_timer);

        while (!ai64_device->timeout && !ai64_device->hw_init_during_read
               && (ai64_device->dma[ai64_device->cbuffer_r].bufferstate ==
                   DmaFree)) {
            DEBUGPx(D_WAIT,
                    "***** WAIT FOR DMA COMPLETE - timeout=%d "
                    "bufferstate=%d wakeup_dma_pending=%d buf_r=%d\n",
                    ai64_device->timeout,
                    ai64_device->dma[ai64_device->cbuffer_r].bufferstate,
                    ai64_device->wakeup_dma_pending,
                    ai64_device->cbuffer_r);
        	
			ai64_device->wakeup_dma_pending = TRUE;	/* 01/10/2014: move it here */
            AI64_UNLOCK(); /** Clear the lock **/
            wait_event_interruptible(ai64_device->dmawq,
                                     (!ai64_device->wakeup_dma_pending));
            AI64_LOCK();    // Lock down while dealing with buffer
            DEBUGPx(D_WAIT, "***** AFTER DMA COMPLETE - timeout=%d "
                    "bufferstate=%d wakeup_dma_pending=%d buf_r=%d\n",
                    ai64_device->timeout,
                    ai64_device->dma[ai64_device->cbuffer_r].bufferstate,
                    ai64_device->wakeup_dma_pending,
                    ai64_device->cbuffer_r);
#if 1 /* DRD-BUG 01/07/2014: If timer has not expired, delete it */
                if (ai64_device->timeout == FALSE)
                    del_timer_sync(&ai64_device->watchdog_timer);
#endif
            if (signal_pending(current)) {
                DEBUGP(D_ERR,
                        "device_read_buffer - WAKEUP w/ SIGNAL PENDING ...\n");
                ai64_device->error = AI64_DEVICE_SIGNAL;
                ai64_abort_any_dma_inprogress(ai64_device);

                if (!ai64_device->timeout)
                    del_timer_sync(&ai64_device->watchdog_timer);

                DEBUGP_EXIT;
                AI64_UNLOCK(); /** Clear the lock **/
                return (-EINTR);
            }
        }

        if (ai64_device->timeout) {
            DEBUGPx(D_ERR, "WAIT FOR DMA TIMEOUT: %d PLX int/ctrl %.8X BC "
                    "%.8X DMA %.8X INT_CTRL %.8X\n",
                    ai64_device->dmainprogress,
                    readl(IntCntrlStat(ai64_device)),
                    readlocal(ai64_device, AI64_BOARD_CTRL_REG),
                    readl(DMACmdStatus(ai64_device)),
                    readlocal(ai64_device, AI64_INT_CTRL_REG));
            ai64_abort_any_dma_inprogress(ai64_device);
            DEBUGP_EXIT;
            AI64_UNLOCK(); /** Clear the lock **/
            return (-ETIME);
        } else {
            DEBUGP(D_DMA, "device_read_continuous - success...\n");
            del_timer_sync(&ai64_device->watchdog_timer);
        }   // End if timeout

        DEBUGPx(D_L4, "buf: %d state=%d\n",
                ai64_device->cbuffer_r,
                ai64_device->dma[ai64_device->cbuffer_r].bufferstate);
        if (ai64_device->hw_init_during_read) {
            DEBUGP(D_ERR,
                    "Hardware initialization occurred "
                    "during read operation\n");
            ai64_device->error = AI64_DEVICE_OPERATION_CANCELLED;
            DEBUGP_EXIT;
            AI64_UNLOCK(); /** Clear the lock **/
            return (-EINTR);
        }

    }

    DEBUGPx(D_L2,
            "**** CONTINUOUS MODE Current cbuffer_r %d / State %d  ****\n",
            ai64_device->cbuffer_r,
            ai64_device->dma[ai64_device->cbuffer_r].bufferstate);

    switch (ai64_device->dma[ai64_device->cbuffer_r].bufferstate) {

    case DmaOverflow:  // The gig is up - we are toast   

        DEBUGP(D_ERR, "*****CONTINUOUS MODE BUFFER OVERFLOW*******\n");

        ai64_device->dma[ai64_device->cbuffer_r].bufferstate = DmaFree;
        ai64_abort_any_dma_inprogress(ai64_device);
        DEBUGP_EXIT;
        AI64_UNLOCK();  // Release me and let me go
        return (-ENOBUFS);
        break;

    case DmaValidData:

        // Mark the buffer in use
        ai64_device->dmabuffer =
            ai64_device->dma[ai64_device->cbuffer_r].dmabufferpool;
        ai64_device->dma[ai64_device->cbuffer_r].bufferstate = DmaInUse;

        /** THERE APPEARS TO BE A SPURIOUS INTERRUPT BEING GENERATED AFTER 
		 ** A MASTER ABORT HAS BEEN ISSUED AND THE FIRST DMA IS REQUESTED.
		 ** THIS IS USUALLY BECAUSE THE MASTER ABORT WAS ISSUED WHEN NO DMA
		 ** WAS PENDING. TO IGNORE THIS SPURIOUS INTERRUPT, SIMPLY SET A FLAG
		 ** 'DEADBEEF' IN THE FIRST SAMPLE OF THE BUFFER AND VERIFY THAT THE 
		 ** SAMPLE WAS NOT UPDATED. IN THAT CASE, RETURN ZERO BYTES TO THE USER.
		 **/
        if (ai64_device->dmabuffer[0] == 0xDEADBEEF) {
            DEBUGPx(D_ERR, "EMPTY DMA READ - buffer number %d!!!\n",
                    ai64_device->cbuffer_r);
            ai64_device->dma[ai64_device->cbuffer_r].bufferstate = DmaFree;
            // Increment the read buffer
            ai64_device->cbuffer_r =
                (++ai64_device->cbuffer_r % ai64_device->num_dma_buffers);
            ai64_hwm_check(ai64_device);    /* Check for HWM */
            DEBUGP_EXIT;
            AI64_UNLOCK();  // Release Lock
            return (0); /* return zero bytes */
            break;
        }

        DEBUGPx(D_DMA, "*** CONTINUOUS MODE VALID DATA ***** Copy %d "
                "Samples into User buffer\n", ai64_device->dmaSampleSize);

        /********************************************************************
		 * VALIDATE CHANNEL 0 REQUEST:                                      *
		 *                                                                  *
		 * If validate channel 0 is requested, it will only be performed    *
		 * if the number of channels is greater than 1. Both channel 0      *
		 * position and not channel 0 positions are validated. Any failure  *
		 * will terminate the read with an IO error.                        *
		 ********************************************************************/
        mode = (readlocal(ai64_device, AI64_BOARD_CTRL_REG) & 
                                AI64_SS_MODE_MASK) >> AI64_SS_MODE_SHIFT;
        ai64_device->nchans =
            1 << (readlocal(ai64_device, AI64_SCAN_SYNCH_CTRL_REG) &
                  AI64_SSCR_CHAN_SIZE_MASK);
        if (mode != AI64_SS_SINGLE_ENDED) {  /* if differential, then half 
                                                number of channels */
            ai64_device->nchans = ai64_device->nchans >> 1;
        }

        /* now check for buffer validation and 16/32 bit xfers */
        AI64_UNLOCK();  // Release Lock
        if((retcode = ai64_validate_data(ai64_device))) {
            return(retcode);
        }
        AI64_LOCK(); /** Set the lock **/

        /*
         * if user has requested more than available, return what
         * ever we have and free up the buffer.
         */

        if (nsamples >= ai64_device->dmaSampleSize) {
            // Release Lock
            AI64_UNLOCK();
            if(__copy_to_user(buf, ai64_device->dmabuffer,
                           ai64_device->dmaSampleSize *
                           ai64_device->bytes_per_sample)) {
                DEBUGP_EXIT;
                return (-EFAULT);
            }
            AI64_LOCK();    // Lock Set
            //
            // Sanity check the data for Debug
            //
            DEBUGPx(D_DMA,
                    "First Data Sample Value 0x%x, "
                    "Last Data Sample Value 0x%x, index %d\n",
                    (unsigned int) ai64_device->dmabuffer[0],
                    (unsigned int) ai64_device->dmabuffer[ai64_device->
                                                          dmaSampleSize -
                                                          1],
                    ai64_device->dmaSampleSize - 1);
            //
            // Mark the buffer as Free
            //
            ai64_device->dma[ai64_device->cbuffer_r].bufferstate = DmaFree;
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples = 0;
            ai64_device->dma[ai64_device->cbuffer_r].dmastart = 0;

            // Increment the read buffer
            DEBUGPx(D_L4, "buf# %d, size=%d\n", ai64_device->cbuffer_r,
                    ai64_device->dmaSampleSize);

            ai64_device->cbuffer_r =
                (++ai64_device->cbuffer_r % ai64_device->num_dma_buffers);

            ai64_hwm_check(ai64_device);    /* Check for HWM */

            DEBUGPx(D_DMA,
                    "*****CONTINUOUS MODE ***** Next cbuffer_r %d\n",
                    ai64_device->cbuffer_r);

            DEBUGPx(D_DMA,
                    "*****CONTINUOUS MODE SUCCESS***** %d samples, Return "
                    "(nsamples %d)\n", ai64_device->dmaSampleSize,
                    nsamples);

            /* now check if the read needs to issue a DMA */
            if (ai64_device->read_to_issue_dma) {
                DEBUGP(D_DMA, "#### READ ISSUING DMA (B) ####\n");
                ai64_fire_dma(ai64_device);
            }
            DEBUGP_EXIT;
            AI64_UNLOCK();  // Release Lock
            return (ai64_device->dmaSampleSize);    // Lets be precise

        } else {
            /*** Return partial buffer and do not free it ***/
            // Release Lock
            AI64_UNLOCK();
            if(__copy_to_user(buf, ai64_device->dmabuffer, nsamples *
                           ai64_device->bytes_per_sample)) {
                DEBUGP_EXIT;
                return (-EFAULT);
            }
            AI64_LOCK();    // Lock Set
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples =
                ai64_device->dmaSampleSize - nsamples;
            ai64_device->dma[ai64_device->cbuffer_r].dmastart = nsamples;
            DEBUGPx(D_DMA, "Partial buffer: dmasamples=%d dmastart=%d\n",
                    ai64_device->dma[ai64_device->cbuffer_r].dmasamples,
                    ai64_device->dma[ai64_device->cbuffer_r].dmastart);
            DEBUGP_EXIT;
            AI64_UNLOCK();  // Release Lock
            return (nsamples);  // return what was requested 
        }

        break;

    case DmaInUse: // Take care of partial DMA buffer
        //
        // Move the data - Must read all the data in the buffer!!
        //
        ai64_device->dmabuffer =
            ai64_device->dma[ai64_device->cbuffer_r].dmabufferpool;
        /*
         * if user has requested more than available, return what
         * ever we have and free up the buffer.
         */
        if (nsamples >=
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples) {
            // Release Lock
            AI64_UNLOCK();

            if (ai64_device->return_16bit_samples) {
                usp = (ushort *) ai64_device->dmabuffer;
                if(__copy_to_user(buf,
                               &usp[ai64_device->
                                    dma[ai64_device->cbuffer_r].dmastart],
                               ai64_device->dma[ai64_device->cbuffer_r].
                               dmasamples * ai64_device->bytes_per_sample)) {
                    DEBUGP_EXIT;
                    return (-EFAULT);
                }
            } else {
                if(__copy_to_user(buf,
                               &ai64_device->dmabuffer[ai64_device->
                                                       dma[ai64_device->
                                                           cbuffer_r].
                                                       dmastart],
                               ai64_device->dma[ai64_device->cbuffer_r].
                               dmasamples * ai64_device->bytes_per_sample)) {
                    DEBUGP_EXIT;
                    return (-EFAULT);
                }
            }

            AI64_LOCK();    // Lock Set

            samples_to_return =
                ai64_device->dma[ai64_device->cbuffer_r].dmasamples;
            //
            // Mark the buffer as Free
            //
            ai64_device->dma[ai64_device->cbuffer_r].bufferstate = DmaFree;
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples = 0;
            ai64_device->dma[ai64_device->cbuffer_r].dmastart = 0;

            // Increment the read buffer
            DEBUGPx(D_L4, "buf# %d, size=%d\n", ai64_device->cbuffer_r,
                    ai64_device->dmaSampleSize);

            ai64_device->cbuffer_r =
                (++ai64_device->cbuffer_r % ai64_device->num_dma_buffers);

            ai64_hwm_check(ai64_device);    /* Check for HWM */

            DEBUGPx(D_DMA,
                    "*****CONTINUOUS MODE ***** Next cbuffer_r %d\n",
                    ai64_device->cbuffer_r);

            /* now check if the read needs to issue a DMA */
            if (ai64_device->read_to_issue_dma) {
                DEBUGP(D_DMA, "#### READ ISSUING DMA (C) ####\n");
                ai64_fire_dma(ai64_device);
            }

            DEBUGP_EXIT;
            AI64_UNLOCK();  // Release Lock
            return (samples_to_return); // return whats left
        } else {
            /*** Return partial buffer and do not free it ***/
            // Release Lock
            AI64_UNLOCK();
            if (ai64_device->return_16bit_samples) {
                usp = (ushort *) ai64_device->dmabuffer;
                if(__copy_to_user(buf,
                               &usp[ai64_device->
                                    dma[ai64_device->cbuffer_r].dmastart],
                               nsamples * ai64_device->bytes_per_sample)) {
                    DEBUGP_EXIT;
                    return (-EFAULT);
                }
            } else {
                if(__copy_to_user(buf,
                               &ai64_device->dmabuffer[ai64_device->
                                                       dma[ai64_device->
                                                           cbuffer_r].
                                                       dmastart],
                               nsamples * ai64_device->bytes_per_sample)) {
                    DEBUGP_EXIT;
                    return (-EFAULT);
                }
            }
            AI64_LOCK();    // Lock Set
            ai64_device->dma[ai64_device->cbuffer_r].dmasamples -=
                nsamples;
            ai64_device->dma[ai64_device->cbuffer_r].dmastart += nsamples;
            DEBUGPx(D_DMA, "Partial buffer: dmasamples=%d dmastart=%d\n",
                    ai64_device->dma[ai64_device->cbuffer_r].dmasamples,
                    ai64_device->dma[ai64_device->cbuffer_r].dmastart);
            DEBUGP_EXIT;
            AI64_UNLOCK();  // Release Lock
            return (nsamples);
        }
        break;


    case DmaFree:  // Still no data in the buffer -  Tell them to wait

        DEBUGPx(D_ERR,
                "*****CONTINUOUS MODE No Available Data  ******* buf=%d\n",
                ai64_device->cbuffer_r);

        // check how much data there is in the FIFO 
        DEBUGPx(D_DMA,
                "AI64_DMA_CONTINUOUS MODE - FIFO Size %d(act) Samples Request: "
                "%d Samples\n", readlocal(ai64_device, AI64_BUFFER_SIZE_REG),
                nsamples);
        DEBUGP_EXIT;
        AI64_UNLOCK();  // Release Lock
        return (-EAGAIN);
        break;

    default:   // Huh??
        DEBUGP(D_ERR,
                "*****CONTINUOUS MODE Corrupt Bufferstate *******\n");
        DEBUGP_EXIT;
        AI64_UNLOCK();  // Release Lock
        return (-EIO);

        break;

    }   // End Switch bufferstate

    DEBUGP(D_ERR, "*****CONTINUOUS MODE Unexpected Return *******\n");
    DEBUGP_EXIT;
    AI64_UNLOCK();  // Release Lock
    return (0);
}

/************************************************************************/
/* Interrupt handler                                                    */
/************************************************************************/
static irqreturn_t 
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
device_interrupt(int irq, void *dev_id, struct pt_regs *regs)
#else
device_interrupt(int irq, void *dev_id)
#endif
{
    struct ai_board *ai64_device = (struct ai_board *) dev_id;
    u32 local_irq_reg;
    u32 plx_irq_reg;
    u32 dma_reg;
    int fifosamples;
    int irq_type;
    int intr_lo_hi_pending = 0;
    int i;
    static int no_request_interrupts = 0;
    static int num_dma = 0;
    // static int num_lohi = 0;

    /* determine if the interrupt is from this board. */

    AI64_LOCK(); /** Set the lock **/

    DEBUGP_ENTER;
    DEBUGPx(D_INT0, "***INTR***: Begins (%d not request)\n",
            no_request_interrupts);
    plx_irq_reg = readl(IntCntrlStat(ai64_device)); // Offset 0x68 (PLX)

    if ((plx_irq_reg & IRQ_PCI_ENABLE) == 0) {  /* not ours */
        DEBUGPx(D_NEVER, "Interrupt not ours... %.8x\n", plx_irq_reg);
#ifdef DEBUG
        int_other_count[ai64_device->board_index]++;
#endif
        DEBUGP(D_INT1, "***INTR***: Returns - Not AI64SS Interrupt\n");
    	AI64_UNLOCK(); /** Unlock **/
    	DEBUGP_EXIT;
    	return IRQ_NONE;
    }

    local_irq_reg = readlocal(ai64_device, AI64_INT_CTRL_REG);   // Offset 0x04

    DEBUGPx(D_INT1,
            "Interrupt->ours! plx irq: %.8X local irq: %.8X plxdma stat: "
            "%.8X \n", plx_irq_reg, local_irq_reg,
            readl(DMACmdStatus(ai64_device)));

/**** DISABLE THIS KLOOGE AS THIS MAY NO LONGER BE REQUIRED ****/
#if 1
    // Check if we came here without a local or DMA interrupt 
    if (!(plx_irq_reg & 0x00200000) && (!(plx_irq_reg & 0x00008000)) &&
        (ai64_device->wait_for_dma_to_complete==0) && (ai64_device->autocal_pending==0)) {
        // Sometimes we get here while waiting for a LO/HI interrupt but 
        // the PLX doesn't indicate it. Check the local device ICR and see 
        // if a LO/HI was pending.  If so - set a flag so that it is handled.

        DEBUGPx(D_INT1,
                "\n\nXXX Interrupt w/o LOCAL or DMA - SHOULD NOT OCCUR XXX "
                "0x%08x(plx), 0x%08x(local) XXXXXXXX\n\n", plx_irq_reg,
                local_irq_reg);
        no_request_interrupts++;
        if ((local_irq_reg & AI64_ICR_1_LO_HI)) {
            intr_lo_hi_pending = 1;
            DEBUGP(D_INT1,
                    "=== Set NO REQUEST - INTR_LO_HI_PENDING 1 === \n");
        } else {
            intr_lo_hi_pending = 0;
#if 0
            /* We need to turn off both interrupts, otherwise, we could hang 
             * the system as no interrupts were cleared
             */
            DEBUGP(D_INT1,"### AI64SS: TURNING OFF INTERRUPTS\n");
            writel(AI64_DMA0_CLR_INT, DMACmdStatus(ai64_device));    
                                                        // Offset 0xA8 (PLX)
            writelocal(ai64_device, 0, AI64_INT_CTRL_REG);
#endif
        }
        DEBUGP(D_INT1, "***INTR***: Returns - Not AI64SS Interrupt\n");
        AI64_UNLOCK(); /** Unlock **/
        DEBUGP_EXIT;
        return IRQ_NONE;
    }
#endif

    /* Check for DMA complete interrupt */
    if (plx_irq_reg & 0x00200000) {

        dma_reg = readl(DMACmdStatus(ai64_device)); // **** Offset 0xA8 (PLX)

        DEBUGPx(D_INT2, "###### DMA COMPLETE ###### plx_irq_reg=%.8X\n",
                dma_reg);

        if (dma_reg & AI64_DMA0_DONE) {

            DEBUGPx(D_INT1,
                    "AI64_DMA0_DONE: ***INTR***: DMA count(%d): "
                    "DMACmdStatus(0xA8) %.8X\n", num_dma, dma_reg);
            num_dma++;

            /* Clear the Interrupt */
            writel(AI64_DMA0_CLR_INT, DMACmdStatus(ai64_device));    // Offset 0xA8 (PLX)

            /* Reset the LO-HI Interrupt if Continuous */
            if (ai64_device->dmaState == AI64_DMA_CONTINUOUS) {
                /* if the current DMA did not reduce the samples down
                 * to below the ai64_device->bufferThreshold, we will
                 * never see a low to high transition, hence, we need to
                 * force a DMA prior to leaving the interrupt routine.
                 */
                fifosamples = readlocal(ai64_device, AI64_BUFFER_SIZE_REG);  // Offset 0x18
                if ((fifosamples) >= ai64_device->bufferThreshold) {// ADJUST??
                    /* Will not get LO/HI Interrupt - Indicate that 
                     * data is there to initiate DMA */
                    no_request_interrupts++;    // Let's count them
                    intr_lo_hi_pending = 1;
                    DEBUGPx(D_INT3,
                            "=== Past THRESHOLD INTR_LO_HI_PENDING 1 "
                            "=== FIFO/Threshold %d/%d\n",
                            fifosamples,
                            (unsigned int) ai64_device->bufferThreshold);
                }
                // Indicate DMA Completed 
                ai64_device->dmainprogress = DMA_COMPLETE;
                DEBUGPx(D_INT2,
                        "Set cbuffer_w %d @ 0x%p to State %d ...\n",
                        ai64_device->cbuffer_w,
                        &ai64_device->dma[ai64_device->cbuffer_w].
                        bufferstate, DmaValidData);

                // Mark the buffer to have DmaValid Data
                ai64_device->dma[ai64_device->cbuffer_w].bufferstate =
                    DmaValidData;
                DEBUGPx(D_L4, "set valid state: buf_w=%d\n",
                        ai64_device->cbuffer_w);

                /*************************************************************
				 ***    T E S T    P A T T E R N    G E N E R A T I O N    ***
				 ***                                                       ***
				 *** If user has requested test pattern generation instead ***
				 *** of actual analog input data, then we overwrite the    ***
				 *** received data with a continuing test pattern.         ***
				 ***                                                       ***
				 *** The test pattern is initialized to zero on INIT or    ***
				 *** device open and is incremented for every sample       ***
				 *************************************************************/
                if (ai64_device->enable_test_pattern) {
                    for (i = 0; i < ai64_device->dmaSampleSize; i++) {
                        ai64_device->dma[ai64_device->cbuffer_w].
                            dmabufferpool[i] = ai64_device->Test_Pattern++;
                        if (ai64_device->return_16bit_samples
                            && (ai64_device->Test_Pattern > 0xffff)) {
                            ai64_device->Test_Pattern = 0;
                        }
                    }
                }
                // Bump up the next write buffer
                ai64_device->cbuffer_w =
                    (++ai64_device->cbuffer_w %
                     ai64_device->num_dma_buffers);

                ai64_hwm_check(ai64_device);    /* Check for HWM */

                DEBUGPx(D_INT2,
                        "dma Incremented cbuffer_w to %d ...\n",
                        ai64_device->cbuffer_w);

                // Wake up processes if waiting and continue on.
                if (ai64_device->wakeup_dma_pending) {

                    DEBUGPx(D_INT1, "DMA Status: Continuous DMA - Wakeup "
                            "blocked process. %.8X\n", dma_reg);

                    /* wake up process blocked in 'read()' if not continuous */
                    ai64_device->wakeup_dma_pending = FALSE;
                    DEBUGP(D_INTW, "Wakeup dmawq\n");
                    wake_up_interruptible(&ai64_device->dmawq);

                }   // End Wakeup Pending

                /* if we already received an Low to High interrupt and were 
                 * waiting for the DMA to complete, this is the time to 
                 * re-issue it by setting the intr_lo_hi_pending flag and 
                 * falling into the lo to high interrupt handler.
                 */
                if (ai64_device->wait_for_dma_to_complete) {
                    ai64_device->wait_for_dma_to_complete = 0;
                    /* re-enable LO TO HIGH interrupt */
                    writelocal(ai64_device, AI64_ICR_1_LO_HI, AI64_INT_CTRL_REG);

                    // Check Fifo Size
                    fifosamples = readlocal(ai64_device, AI64_BUFFER_SIZE_REG);
                    // Offset 0x18
                    if ((fifosamples) >= ai64_device->bufferThreshold) {    // ADJUST??
                        intr_lo_hi_pending = 1;
                    }
                }
            } else {
                /*************************************************************
				 ***    T E S T    P A T T E R N    G E N E R A T I O N    ***
				 ***                                                       ***
				 *** If user has requested test pattern generation instead ***
				 *** of actual analog input data, then we overwrite the    ***
				 *** received data with a continuing test pattern.         ***
				 ***                                                       ***
				 *** The test pattern is initialized to zero on INIT or    ***
				 *** device open and is incremented for every sample       ***
				 *************************************************************/
                if (ai64_device->enable_test_pattern) {
                    for (i = 0; i < ai64_device->dmaSampleSize; i++) {
                        ai64_device->dmabuffer[i] = ai64_device->Test_Pattern++;
                        if (ai64_device->return_16bit_samples
                            && (ai64_device->Test_Pattern > 0xffff)) {
                            ai64_device->Test_Pattern = 0;
                        }
                    }
                }

                DEBUGPx(D_INT2,
                        "***INTR***: DMA Status: Normal DMA - Wakeup "
                        "Process blocked. %.8X\n", dma_reg);

                /* wake up process blocked in 'read()' if not continuous */
                ai64_device->dmainprogress = NO_DMA_INPROGRESS;
                ai64_device->wakeup_dma_pending = FALSE;
                DEBUGP(D_INTW, "Wakeup dmawq\n");
                //wake_up(&ai64_device->dmawq);
                wake_up_interruptible(&ai64_device->dmawq);

                DEBUGPx(D_INT2,
                        "***INTR***: DMA end return plx irq %.8X local irq %.8X. "
                        "plxdma stat%.8X.\n",
                        readl(IntCntrlStat(ai64_device)),
                        readlocal(ai64_device, AI64_INT_CTRL_REG),
                        readl(DMACmdStatus(ai64_device)));

                DEBUGP(D_INT1, "***INTR***: Return DMA 0\n");
                goto ENABLE_IRQ;
            }
        }
    }

    /*
     *  handle group 0 interrupts.
     */
    DEBUGPx(D_INT0, "Before Group 0 interrupt, local irq reg %.8x\n",
            local_irq_reg);

    if (local_irq_reg & AI64_ICR_0_REQUEST) {
        irq_type = local_irq_reg & AI64_ICR_0_MASK;
        //writelocal(ai64_device,local_irq_reg & (~(IRQ_0_REQUEST|IRQ_0_MASK)), 
        // AI64_INT_CTRL_REG);
        writelocal(ai64_device, 0, AI64_INT_CTRL_REG);
        DEBUGPx(D_INT0, "Group 0 interrupt. type: %x\n", irq_type);

        switch (irq_type) {
        case AI64_ICR_0_IDLE:
            DEBUGPx(D_INT0, "Group 0 IRQ_0_IDLE Interrupt %x\n", irq_type);
            if (ai64_device->init_pending) {
                ai64_device->init_pending = FALSE;
                DEBUGP(D_INTW, "wakeup ioctlwq\n");
                wake_up(&ai64_device->ioctlwq);
            }
            break;
        case AI64_ICR_0_AUTOCAL_COMPLETE:
            DEBUGPx(D_INT0,
                    "Group 0 IRQ_0_AUTOCAL_COMPLETE Interrupt %x\n",
                    irq_type);
            if (ai64_device->autocal_pending) {
                ai64_device->autocal_pending = FALSE;
                DEBUGP(D_INTW, "wakeup ioctlwq\n");
                wake_up(&ai64_device->ioctlwq);
            }
            break;
        case AI64_ICR_0_INPUT_SCAN_INIT:
            DEBUGPx(D_INT0,
                    "Group 0 IRQ_0_INPUT_SCAN_INIT Interrupt %x\n",
                    irq_type);
            if (ai64_device->init_pending) {
                ai64_device->init_pending = FALSE;
                DEBUGP(D_INTW, "wakeup ioctlwq\n");
                wake_up(&ai64_device->ioctlwq);
            }
            break;
        case AI64_ICR_0_INPUT_SCAN_COMPLETE:
            DEBUGP(D_INT0, "channels ready IRQ\n");
            if (ai64_device->chready_pending) {
                DEBUGP(D_INT0, "channels ready IRQ was waited for\n");
                ai64_device->chready_pending = FALSE;
                DEBUGP(D_INTW, "wakeup dmawq\n");
                wake_up(&ai64_device->dmawq);
            }
            break;
        default:
            DEBUGP(D_ERR, "unknown group 0 Interrupt\n");
            break;
        }
    }

    /*
     *  handle group 1 interrupts.
     */
    if ((local_irq_reg & AI64_ICR_1_REQUEST) || (intr_lo_hi_pending)) {
        DEBUGPx(D_INT1,
                "Group 1 Interrupt Handling  ********: local_irq_reg=0x%x\n",
                local_irq_reg);

        //  writelocal(ai64_device,local_irq_reg & (~(IRQ_1_REQUEST|IRQ_1_MASK)), 
        // AI64_INT_CTRL_REG);
        writelocal(ai64_device, 0, AI64_INT_CTRL_REG);
        // Clear the local Interrupt Control Reg.


        /* We need to re-enable the interrupt otherwise we may
         * lose an interrupt if enabled after the number of samples
         * had exceeded the threshold value (as is being done in
         * the device_read_continuous() routine.
         */
        writelocal(ai64_device, AI64_ICR_1_LO_HI, AI64_INT_CTRL_REG);

        irq_type = local_irq_reg & AI64_ICR_1_MASK;

        if (intr_lo_hi_pending) {
            // Set the type to LO_HI 
            irq_type = AI64_ICR_1_LO_HI;
        }

        switch (irq_type) {

        case AI64_ICR_1_LO_HI:
            DEBUGP(D_INT1,
                    "AI64_ICR_1_LO_HI: ****** Interrupt Handling ******\n");

            if (ai64_device->lo_hi_pending) {
                ai64_device->lo_hi_pending = FALSE;
                DEBUGP(D_INT2,
                        "AI64_ICR_1_LO_HI: ****** lo_hi_pending set FALSE ******\n");

                if (intr_lo_hi_pending) {
                    intr_lo_hi_pending = 0;
                    DEBUGP(D_INT1,
                            "AI64_ICR_1_LO_HI: ****** intr_lo_hi_pending "
                            "set FALSE ******\n");
                } else {
                    ai64_device->wakeup_lohi_pending = TRUE;
                    DEBUGP(D_INT1,
                            "AI64_ICR_1_LO_HI:  ** intr_lo_hi_pending FALSE "
                            "/ Wakeup Pending TRUE \n");
                }

                // break;  We may also be in continous mode
            }
            //
            // NEW - DMA Continuous dictates we DMA the current FIFO into the 
            // next buffer
            //

            if (ai64_device->dmaState == AI64_DMA_CONTINUOUS) {  // Queue up next DMA 
                // Check state of next buffer
                if (ai64_device->hw_init_during_read
                    || ai64_device->dma_abort_request) {
                    ai64_device->dmainprogress = NO_DMA_INPROGRESS;
                    DEBUGP(D_ERR,
                            "HW Initialized or DMA abort request prior "
                            "to starting DMA\n");
                    break;
                }
                // DEBUGP(D_INT1,"AI64_DMA_CONTINUOUS:  ***** Check BufferState "
                //      "***** \n");

                switch (ai64_device->dma[ai64_device->cbuffer_w].
                        bufferstate) {

                case DmaValidData: // We have a buffer overflow
                case DmaInUse: // We have a buffer overflow
                    fifosamples = readlocal(ai64_device, AI64_BUFFER_SIZE_REG);
                    if (fifosamples >= (MAX_DMA_SAMPLES - 1)) {
                        DEBUGPx(D_INTE, "*** Set BUFFER OVERFLOW/DISABLE "
                                "*** fifosamples=%d\n", fifosamples);
                        ai64_device->dmaoverflow = TRUE;
                        ai64_device->dma[ai64_device->cbuffer_w].
                            bufferstate = DmaOverflow;
                    } else
                        ai64_device->read_to_issue_dma++;
                    break;

                case DmaOverflow:  // Should never be here
                    DEBUGP(D_INTE,
                            "AI64_DMA_CONTINUOUS: bufferstate Overflowed Buffer?\n");
                    break;

                case DmaFree:  // Clean buffer - Fill'er up!
                    DEBUGPx(D_BUFF,
                            "AI64_DMA_CONTINUOUS: Free Buf Physical Address "
                            "dmaoffset[%d]=0x%08lx, Fifosize 0x%x\n",
                            ai64_device->cbuffer_w,
                            ai64_device->dma[ai64_device->cbuffer_w].
                            dmaoffset, readlocal(ai64_device,
                                                 AI64_BUFFER_SIZE_REG));

                    /* If DMA is in progress, wait for it to complete before 
                     * firing the next DMA
                     */
                    if (ai64_device->dmainprogress == DMA_INPROGRESS) {
                        /* DISABLE LO TO HIGH interrupt */
                        writelocal(ai64_device, 0, AI64_INT_CTRL_REG);
                        ai64_device->wait_for_dma_to_complete++;
                        DEBUGPx(D_L4, "#### WAIT FOR DMA TO COMPLETE ####"
                                "BufCtrl=0x%x FIFO=0x%x cbuffer_w=%d bufferstate=%d\n",
                                readlocal(ai64_device,
                                          AI64_INPUT_BUFF_CTRL_REG),
                                readlocal(ai64_device, AI64_BUFFER_SIZE_REG),
                                ai64_device->cbuffer_w,
                                ai64_device->dma[ai64_device->cbuffer_w].
                                bufferstate);
                        break;
                    }
                    if ((ai64_device->dmaSampleSize >
                         2 * ai64_device->nchans)
                        && (readlocal(ai64_device, AI64_BUFFER_SIZE_REG) <
                            (ai64_device->dmaSampleSize -
                             2 * ai64_device->nchans))) {

                        writelocal(ai64_device, AI64_ICR_1_LO_HI, AI64_INT_CTRL_REG);
                        /* enable interrupts */
                        DEBUGPx(D_L3,
                                "### WAIT FOR SAMPLES: fifo_read=0x%x "
                                "dmaSampleSize=0x%x\n",
                                readlocal(ai64_device, AI64_BUFFER_SIZE_REG),
                                ai64_device->dmaSampleSize);
                    } else {
                        DEBUGP(D_DMA,
                                "#### INTERRUPT ISSUING DMA ####\n");
                        ai64_fire_dma(ai64_device);
                        // fire the dma if samples available
                    }
                    break;


                default:
                    DEBUGP(D_ERR, "dma bufferstate corrupt!!\n");
                    ai64_device->error = AI64_DEVICE_DMA_BUFFER;
                    DEBUGP(D_INT1,
                            "AI64_DMA_CONTINUOUS:  ******* DMA Disabled ******** \n");
                    break;


                }   // End Switch bufferstate

            }   // End if DmaState Continuous  


            if (ai64_device->wakeup_lohi_pending) {
                ai64_device->wakeup_lohi_pending = FALSE;
                DEBUGP(D_INTW, "wakeup lohiwq\n");
                wake_up(&ai64_device->lohiwq);
                DEBUGP(D_INT1,
                        "AI64_ICR_1_LO_HI:  Wake up issued to those waited for LO/HI\n");
            } else {
                DEBUGP(D_INT1,
                        "dma icr1 lo-hi: LO-HI Not Pending / No dma continuous\n");
            }

            break;  // AI64_ICR_0_LO_HI



        case AI64_ICR_1_HI_LO:
            DEBUGPx(D_NEVER, ": IRQ_1_HI_LO 1 Interrupt %x\n", irq_type);
            break;
        }   // End Switch irq_type
    }   // End if  AI64_ICR_1_Request

    /* if something is pending, leave with the interrupt enabled. */

#if 0   // DRD
    if (ai64_device->dmainprogress || ai64_device->autocal_pending
        || ai64_device->chready_pending || ai64_device->lo_hi_pending
        || ai64_device->init_pending
        || ai64_device->dmaState == AI64_DMA_CONTINUOUS) {
        writel(readl(IntCntrlStat(ai64_device)) | IRQ_PCI_ENABLE |
               IRQ_LOCAL_PCI_ENABLE, IntCntrlStat(ai64_device));
    }
#endif

  ENABLE_IRQ:

    AI64_UNLOCK(); /** Unlock **/
    DEBUGP(D_INT1, "***INTR***: Return Bottom \n");
    DEBUGP_EXIT;
    return IRQ_HANDLED;
}

/************************************************************************/
/* writelocal                                                           */
/*                                                                      */
/* Write to the board local registers in memory space.                  */
/*                                                                      */
/*                                                                      */
/************************************************************************/
void writelocal(struct ai_board *ai64_device, unsigned value,
                unsigned address)
{
    writel(value, ai64_device->local_reg_address + address);
};

/************************************************************************/
/* readlocal                                                            */
/*                                                                      */
/* Read from the board local registers in memory space.                 */
/*                                                                      */
/*                                                                      */
/************************************************************************/
unsigned readlocal(struct ai_board *ai64_device, unsigned address)
{
    return readl(ai64_device->local_reg_address + address);
};


/***
 ***  Formerly pci16AI64_IOCTL.C 
 ***
 ***  General description of the ioctl code below:
 ***  	Device driver source code for General Standards PCI-16AI64 
 ***  	64 channel PCI A/D board. This file is part of the Linux
 ***  	driver source distribution for this board.
 ***  	
 ***  	This file is not necessary to compile application programs, therefore 
 ***  	should NOT be included in binary only driver distributions.
 ***
 ***  Copyrights (c):
 ***  	General Standards Corporation (GSC), 2002-2003
 ***  	Concurrent Computer Corporation, 2003-2004
 ***
 ***  Author:
 ***  	Evan Hillman, GSC Inc. (evan@generalstandards.com)
 ***
 ***  Updated:
 ***  	Rick Calabro, Concurrent Computer Corp 
 ***  
 ***  Support:
 ***  	Primary support for this driver is provided by CCC. 
 ***
 ***  Platform (tested on, may work with others):
 ***  	RedHawk Linux, kernel version 2.6.x, iHawk platforms only.
 ***
 ***  Modified:
 ***    10/22/03: Rick Calabro - DMA Buffer Not Cleared
 ***    12/08/03: Rick Calabro - DMA Buffer Size corrected 
 ***
 ***/

#define IOCTL_RETURN(Code) {     \
    ai64_device->ioctl_processing = 0; \
    DEBUGP_EXIT;    \
    return (Code);  \
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
long unlocked_device_ioctl(struct file *filp,u_int iocmd,unsigned long ioarg )
{
    int ret;
    struct inode *inode;
    struct ai_board *ai64_device = (struct ai_board *) filp->private_data;

    DEBUGP_ENTER;
    DEBUGPx(D_L2, "device pointer: 0x%p\n", ai64_device);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
    inode = filp->f_dentry->d_inode;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
    inode = filp->f_path.dentry;
#else
    inode = filp->f_path.dentry->d_inode;
#endif

    mutex_lock(&ai64_device->ioctl_mtx);
    if(ai64_device->ioctl_processing) {
        DEBUGP(D_ERR,"ioctl processing active: busy\n");
        DEBUGP_EXIT;
    	mutex_unlock(&ai64_device->ioctl_mtx);
        return (-EBUSY);    /* DO NOT USE IOCTL_RETURN() CALL HERE */
    }
    mutex_unlock(&ai64_device->ioctl_mtx);

    ret = device_ioctl(inode, filp, iocmd, ioarg);

    return( (long) ret );
}
#endif

/* ioctl file operation: this does the bulk of the work */
int device_ioctl(struct inode *inode, struct file *fp, unsigned int num,
                 unsigned long arg)
{
    struct ai_board *ai64_device = (struct ai_board *) fp->private_data;
    ai64_set_aux_sync_io_t aux_sync_io;
    unsigned long ulval, regval;
    unsigned long mmap_reg_select = 0;
	ai64_mmap_select_t	ai64_mmap_select;
    struct ai64_registers registers;
    struct ai64_hwm buf_hwm;
    unsigned long bindaddr, bindoffset=0;
    ai64_phys_mem_t ai64_phys_mem;
    int i, j, count, region;
    u32 plx_irq_reg;
    int     retry;
	int 	InputClearBuffer;

    DEBUGP_ENTER;
    DEBUGPx(D_L2, "device pointer: 0x%p\n", ai64_device);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
    if(ai64_device->ioctl_processing) {
        DEBUGP(D_ERR,"ioctl processing active: busy\n");
        DEBUGP_EXIT;
        return (-EBUSY);    /* DO NOT USE IOCTL_RETURN() CALL HERE */
    }
#endif

    ai64_device->ioctl_processing++;

    /* main ioctl function dispatch */
    /* 'break' at the end of branches which need to wait for the */
    /* channels ready condition, 'return' from others */
    switch (num) {

    case IOCTL_AI64_NO_COMMAND:
        DEBUGP(D_IOCTL, "IOCTL_AI64_NO_COMMAND\n");
        IOCTL_RETURN(0);

    case IOCTL_AI64_ABORT_READ:
    case IOCTL_AI64_INIT_BOARD:

        ai64_device->hw_init_during_read++; /* set flag incase read oper */
        ai64_device->Test_Pattern = 0;  /* for debug only */
        ai64_device->chan0_index = 0;

        if (num == IOCTL_AI64_ABORT_READ)
            DEBUGP(D_IOCTL, "IOCTL_AI64_ABORT_READ\n");
        else
            DEBUGP(D_IOCTL, "IOCTL_AI64_INIT_BOARD\n");

        /* disable generator A */
        regval = readlocal(ai64_device, AI64_RATE_A_GENERATOR_REG);
        regval |= AI64_RGR_GEN_DISABLE;
        writelocal(ai64_device, regval, AI64_RATE_A_GENERATOR_REG);

        /* disable generator B */
        regval = readlocal(ai64_device, AI64_RATE_B_GENERATOR_REG);
        regval |= AI64_RGR_GEN_DISABLE;
        writelocal(ai64_device, regval, AI64_RATE_B_GENERATOR_REG);

        REG_DUMP(D_REGS, ai64_device, "IOCTL_AI64_INIT_BOARD");

        /* abort any DMA in-progress */
        ai64_abort_any_dma_inprogress(ai64_device);

        ai64_device->init_pending = 1;

        /* wakeup any pending interrupts */
        if (ai64_device->chready_pending) {
            ai64_device->chready_pending = FALSE;
            ai64_device->wakeup_dma_pending = FALSE;
            DEBUGP(D_WAIT, "Wakeup dmawq\n");
            wake_up(&ai64_device->dmawq);
            ai64_device->init_pending++;
        }

        if (ai64_device->lo_hi_pending) {
            ai64_device->lo_hi_pending = FALSE;
            ai64_device->wakeup_lohi_pending = FALSE;
            DEBUGP(D_WAIT, "Wakeup lohiwq\n");
            wake_up(&ai64_device->lohiwq);
            ai64_device->init_pending++;
        }

        if (ai64_device->dmainprogress) {
            ai64_device->dmainprogress = NO_DMA_INPROGRESS;
            ai64_device->wakeup_dma_pending = FALSE;
            DEBUGP(D_WAIT, "Wakeup dmawq\n");
            wake_up(&ai64_device->dmawq);
            ai64_device->init_pending++;
        }

        if (ai64_device->wakeup_dma_pending) {
            ai64_device->wakeup_dma_pending = FALSE;
            DEBUGP(D_WAIT, "Wakeup dmawq\n");
            wake_up(&ai64_device->dmawq);
            ai64_device->init_pending++;
        }

        /* if read still in progress, wait for completion otherwise
         * we could lock up the driver */
        retry=100;
        while(ai64_device->read_in_progress && retry--) {
            DEBUGP(D_WAIT, "sleep for 10 ms (ioctlwq block)\n");
            wait_event_interruptible_timeout(ai64_device->ioctlwq,0,
                                           MSECS_TO_SLEEP(10));
            DEBUGP(D_WAIT, "wake_up after sleep\n");
        }

        if(ai64_device->read_in_progress) {
            DEBUGP(D_ERR, "Something Wrong! Read in progress\n");
            IOCTL_RETURN(-EBUSY);
        }

        ai64_device->read_in_progress = 0;
        ai64_device->ReadCount = 0;
        ai64_device->WriteCount = 0;
        ai64_device->timestamp_read_hwm = 0;
        ai64_device->timestamp_btw_read_hwm = 0;

        AI64_LOCK();
        /* if we need to wait for completion */
        if (ai64_device->init_pending > 1) {
            DEBUGP(D_WAIT, "sleep for 10 ms (ioctlwq block)\n");
            AI64_UNLOCK();
            wait_event_interruptible_timeout(ai64_device->ioctlwq,0,
                                           MSECS_TO_SLEEP(10));
            AI64_LOCK();
            DEBUGP(D_WAIT, "wake_up after sleep\n");
        }

        /* abort any DMA in-progress */
        ai64_abort_any_dma_inprogress(ai64_device);

        /*
         *    clear out any pending old interrupts.
         */
        writelocal(ai64_device, 0, AI64_INT_CTRL_REG);
        regval = readl(IntCntrlStat(ai64_device));
        regval &= ~PCI_INT_ENABLE;
        writel(regval, (IntCntrlStat(ai64_device)));

        /* enable PLX interrupt. */
        writel(IRQ_PCI_ENABLE | IRQ_LOCAL_PCI_ENABLE | IRQ_LOCAL_ENABLE,
               IntCntrlStat(ai64_device));

        /* enable local init complete interrupt. */
        //writelocal(ai64_device,IRQ_0_IDLE|IRQ_0_REQUEST, AI64_INT_CTRL_REG);

        /* if board initialize request, initialize the board */
        if (num == IOCTL_AI64_INIT_BOARD) {
            ai64_device->timeout = FALSE;
            ai64_device->watchdog_timer.expires =
                jiffies + ai64_device->timeout_seconds * HZ;
            add_timer(&ai64_device->watchdog_timer);

            writelocal(ai64_device, AI64_BCR_INITIALIZE, AI64_BOARD_CTRL_REG);

            DEBUGPx(D_NEVER, "init ->plx int %.8X board control %.8X "
                    "local int %.8X\n", readl(IntCntrlStat(ai64_device)),
                    readlocal(ai64_device, AI64_BOARD_CTRL_REG),
                    readlocal(ai64_device, AI64_INT_CTRL_REG));

            DEBUGPx(D_WAIT,
                    "Wait for interrupt: ioctlwq: init_pending=%d\n",
                    ai64_device->init_pending);

            AI64_UNLOCK();
            wait_event_interruptible(ai64_device->ioctlwq,
                                     (!ai64_device->init_pending));
            AI64_LOCK();
#if 0
            /* The new 16AI64SS/C has the default for DISABLE DEMAND MODE changed to enabled */
            regval = readlocal(ai64_device, AI64_BOARD_CTRL_REG);
	        regval &= ~0x00080000;	/* Clear Disable Demand Mode */
            writelocal(ai64_device, regval, AI64_BOARD_CTRL_REG);
#endif

            DEBUGPx(D_WAIT, "Wait over: ioctlwq: init_pending=%d\n",
                    ai64_device->init_pending);

            if (ai64_device->timeout) {
                DEBUGP(D_ERR, "timeout during board initialize\n");
                ai64_device->init_pending = FALSE;
                ai64_device->error = AI64_DEVICE_IOCTL_TIMEOUT;
                AI64_UNLOCK();
                IOCTL_RETURN(-ETIME);
            } else
                del_timer_sync(&ai64_device->watchdog_timer);
        }

        /* reset init_pending flag */
        ai64_device->init_pending++;

        /* Clean up buffers */
        ai64_device->dmaoverflow = FALSE;

        /*** DO NOT LOCK WHILE CLEARING THE BUFFERS AS THIS TAKES SOME
		 *** FINITE TIME. THE RESULT IS THAT IF WE HAVE AN APPLICATION
		 *** COLLECTING DATA AT THE MAXIMUM CLOCK RATE ON A BOARD WHILE
		 *** ANOTHER BOARD ISSUES A INITIALIZATION, WE COULD POSSIBLY
		 *** CAUSE THE SAMPLES TO OVERFLOW ON THE BOARD COLLECTING DATA.
		 ***/
        AI64_UNLOCK();
        // Zero out the buffers
        for (i = 0; i < ai64_device->num_dma_buffers; i++) {
            ai64_device->dmabuffer = ai64_device->dma[i].dmabufferpool;
            ai64_device->dma[i].bufferstate = DmaFree;
            ai64_device->dma[i].dmasamples = 0;
            ai64_device->dma[i].dmastart = 0;

            for (j = 0; j < DMA_SAMPLES; j++) {
                ai64_device->dmabuffer[j] = 0xDEADBEEF;
            }
        }

        AI64_LOCK();
        ai64_device->cbuffer_r = 0;
        ai64_device->cbuffer_w = 0;
        ai64_device->buf_inuse_hwm = 0;
        ai64_device->dmainprogress = NO_DMA_INPROGRESS;
        ai64_device->error = AI64_DEVICE_SUCCESS;
        ai64_device->nchans =
            1 << (readlocal(ai64_device, AI64_SCAN_SYNCH_CTRL_REG) &
                  AI64_SSCR_CHAN_SIZE_MASK);

        REG_DUMP(D_NEVER, ai64_device, "IOCTL_AI64_INIT_BOARD");
        DEBUGPx(D_NEVER,
                "IntCntrlStat=%.8X AI64_BOARD_CTRL_REG=%.8X DMACmdStatus=%.8X\n",
                readl(IntCntrlStat(ai64_device)), readlocal(ai64_device,
                                                    AI64_BOARD_CTRL_REG),
                readl(DMACmdStatus(ai64_device)));
        DEBUGPx(D_NEVER,
                "plx int %.8X board control %.8X local int %.8X\n",
                readl(IntCntrlStat(ai64_device)), readlocal(ai64_device,
                                                    AI64_BOARD_CTRL_REG),
                readlocal(ai64_device, AI64_INT_CTRL_REG));

        DEBUGP(D_L2, "ioctl initalize done\n");
        AI64_UNLOCK();

        /* clear init_pending flag */
        ai64_device->init_pending = FALSE;

        IOCTL_RETURN(0);

    case IOCTL_AI64_START_AUTOCAL:
        DEBUGP(D_IOCTL, "IOCTL_AI64_START_AUTOCAL\n");

        if (ai64_device->read_in_progress) {
            DEBUGP(D_ERR, "Read in progress\n");
            IOCTL_RETURN(-EBUSY);
        }

        AI64_LOCK();
        DEBUGPx(D_NEVER,
                "plx int %.8X board control %.8X local int %.8X\n",
                readl(IntCntrlStat(ai64_device)), readlocal(ai64_device,
                                                    AI64_BOARD_CTRL_REG),
                readlocal(ai64_device, AI64_INT_CTRL_REG));
        DEBUGPx(D_NEVER, "base_address[0]=0x%lX, base_address[2]=0x%lX\n",
                (unsigned long)ai64_device->pdev->resource[0].start,
                (unsigned long)ai64_device->pdev->resource[2].start);

        /* enable local interrupt. */
        regval = readlocal(ai64_device, AI64_INT_CTRL_REG);
        regval &= ~AI64_ICR_0_MASK;
        regval |= AI64_ICR_0_AUTOCAL_COMPLETE;
        ai64_device->autocal_pending = TRUE;
        writelocal(ai64_device, regval, AI64_INT_CTRL_REG);

        /* start autocal */
        regval = readlocal(ai64_device, AI64_BOARD_CTRL_REG);
//    regval &= ~(AI64_ICR_0_MASK);
        regval |= AI64_BCR_AUTOCAL;
        ai64_device->autocal_pending = TRUE;
        /* enable PLX interrupt. */
        writel(readl(IntCntrlStat(ai64_device)) | IRQ_PCI_ENABLE |
               IRQ_LOCAL_PCI_ENABLE | IRQ_LOCAL_ENABLE,
               IntCntrlStat(ai64_device));
        writelocal(ai64_device, regval, AI64_BOARD_CTRL_REG);

        /* wait for complete */
        ai64_device->timeout = FALSE;
        ai64_device->watchdog_timer.expires =
            jiffies + ai64_device->timeout_seconds * HZ;
        add_timer(&ai64_device->watchdog_timer);

        writelocal(ai64_device, regval, AI64_BOARD_CTRL_REG);

        DEBUGPx(D_WAIT,
                "Wait for interrupt: ioctlwq: autocal_pending=%d\n",
                ai64_device->lo_hi_pending);
        AI64_UNLOCK();
        wait_event_interruptible(ai64_device->ioctlwq,
                                 (!ai64_device->autocal_pending));
        AI64_LOCK();
        DEBUGPx(D_WAIT, "Wait over: ioctlwq: autocal_pending=%d\n",
                ai64_device->lo_hi_pending);

        if (ai64_device->timeout) {
            DEBUGP(D_ERR, "timeout when calibrating board\n");
            ai64_device->autocal_pending = FALSE;
            ai64_device->error = AI64_DEVICE_IOCTL_TIMEOUT;
            AI64_UNLOCK();
            IOCTL_RETURN(-ETIME);
        } else
            del_timer_sync(&ai64_device->watchdog_timer);

        DEBUGP(D_L2, "ioctl autocal done\n");
        AI64_UNLOCK();
        IOCTL_RETURN(0);

    case IOCTL_AI64_GET_DEVICE_TYPE:
        DEBUGP(D_IOCTL, "IOCTL_AI64_GET_DEVICE_TYPE\n");
        put_user_ret(ai64_device->board_type, (unsigned long *) arg,
                     (-EFAULT));
        IOCTL_RETURN(0);

    case IOCTL_AI64_GET_MASTER_CLOCK_FREQUENCY:
        DEBUGP(D_IOCTL, "IOCTL_AI64_GET_MASTER_CLOCK_FREQUENCY\n");
        put_user_ret(ai64_device->MasterClockFreq, (double *) arg,
                     (-EFAULT));
        IOCTL_RETURN(0);

    case IOCTL_AI64_GET_FIRMWARE_REV:    // Added <RAC>
        DEBUGP(D_IOCTL, "IOCTL_AI64_GET_FIRMWARE_REV\n");
        DEBUGPx(D_L2, "Firmware Revision: 0x%x \n", ai64_device->firmware);
        put_user_ret((unsigned long)ai64_device->firmware, 
                                    (unsigned long *) arg, (-EFAULT));
        IOCTL_RETURN(0);

    case IOCTL_AI64_READ_REGISTER:
        DEBUGP(D_IOCTL, "IOCTL_AI64_READ_REGISTER entry ...\n");
        /* Copy the register structure from the user or application */
        if(copy_from_user(&registers, (void *) arg, sizeof(registers))) {
            IOCTL_RETURN(-EFAULT);
        }

        /* Check if the argument passed is valid */

        if (AI64_REGISTER_TYPE(registers.ai64_register) ==
            AI64_GSC_REGISTER) {
            if ((registers.ai64_register > AI64_GSC_AUXCTRL)) {
                /* Register number that is to be read is invalid */
                ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
                DEBUGPx(D_ERR, "invalid ai64_register=0x%x\n",
                        registers.ai64_register);
                IOCTL_RETURN(-EINVAL);
            }

            /* Read the register content from the specified register */

            registers.register_value =
                readl(ai64_device->local_reg_address +
                      AI64_REGISTER_INDEX(registers.ai64_register));
            DEBUGPx(D_L2, "Register value read = %8.8X\n",
                    registers.register_value);
        } else
            if (AI64_REGISTER_TYPE(registers.ai64_register) ==
                AI64_PLX_REGISTER) {
            if ((registers.ai64_register < AI64_PLX_LASORR)
                || (registers.ai64_register > AI64_PLX_LBRD1)) {
                /* Local config register to be written is Invalid */
                ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
                DEBUGPx(D_ERR, "invalid ai64_register=0x%x\n",
                        registers.ai64_register);
                IOCTL_RETURN(-EINVAL);
            }
            registers.register_value =
                readl(ai64_device->config_reg_address +
                      AI64_REGISTER_INDEX(registers.ai64_register));
        } else if (AI64_REGISTER_TYPE(registers.ai64_register) ==
                   AI64_PCI_REGISTER) {
            if ((registers.ai64_register < AI64_PCI_IDR)
                || (registers.ai64_register > AI64_PCI_ILR_IPR_MGR_MLR)) {

                /* Pci config register cannot be written */
                ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
                DEBUGPx(D_ERR, "invalid ai64_register=0x%x\n",
                        registers.ai64_register);
                IOCTL_RETURN(-EINVAL);
            }
            /*** read the correct configuration location - drd ***/
            pci_read_config_dword(ai64_device->pdev, (unsigned long)
                                  AI64_REGISTER_INDEX(registers.
                                                      ai64_register) *
                                  (sizeof(u32)),
                                  &registers.register_value);
        } else {
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            DEBUGPx(D_ERR, "invalid ai64_register=0x%x\n",
                    registers.ai64_register);
            IOCTL_RETURN(-EINVAL);
        }

        /* write the register read content to the structure and
           copy to user space */
        if(copy_to_user((void *) arg, &registers, sizeof(registers))) {
            IOCTL_RETURN(-EFAULT);
        }

        IOCTL_RETURN(0);

    case IOCTL_AI64_WRITE_REGISTER:
        DEBUGP(D_IOCTL, "IOCTL_AI64_WRITE_REGISTER entry ...\n");
        /* Copy the register structure from the user/application */
        if(copy_from_user(&registers, (void *) arg, sizeof(registers))) {
            IOCTL_RETURN(-EFAULT);
        }

        /* Check if the argument passed is valid */
        if (AI64_REGISTER_TYPE(registers.ai64_register) ==
            AI64_GSC_REGISTER) {
            goto gsc_register_write;
        } else
            if (AI64_REGISTER_TYPE(registers.ai64_register) ==
                AI64_PCI_REGISTER) {
            goto pci_register_write;
        }

        else if (AI64_REGISTER_TYPE(registers.ai64_register) ==
                 AI64_PLX_REGISTER) {
            goto plx_register_write;
        } else {
            /* Invalid register type. */
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            DEBUGPx(D_ERR, "invalid ai64_register=0x%x\n",
                    registers.ai64_register);
            IOCTL_RETURN(-EFAULT);
        }

      gsc_register_write:
        if ((registers.ai64_register > AI64_GSC_AUXCTRL) ||
            (registers.ai64_register == AI64_GSC_IDBR) ||
            (registers.ai64_register == AI64_GSC_BUFSZ) ||
            (registers.ai64_register == AI64_GSC_BCFG)) {
            /* Register number that is to be read is invalid */
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            DEBUGPx(D_ERR, "invalid ai64_register=0x%x\n",
                    registers.ai64_register);
            IOCTL_RETURN(-EINVAL);
        }
        DEBUGPx(D_L2, "REG VALUE : %8.8X\n",
                readl(ai64_device->local_reg_address
                      + AI64_REGISTER_INDEX(registers.ai64_register)));

        /* Write the value to the specified register */
        writel(registers.register_value, ai64_device->local_reg_address +
               AI64_REGISTER_INDEX(registers.ai64_register));
        DEBUGPx(D_L2, "REG VALUE after Writing: %8.8X\n",
                readl(ai64_device->local_reg_address
                      + AI64_REGISTER_INDEX(registers.ai64_register)));
        IOCTL_RETURN(0);

      pci_register_write:
        DEBUGP(D_IOCTL, "pci_register_write entry ...\n");
        if ((registers.ai64_register < AI64_PCI_CR_SR)
            || (registers.ai64_register > AI64_PCI_ILR_IPR_MGR_MLR)) {
            /* Pci config register cannot be written */
            DEBUGP(D_ERR, "Pci config register cannot be written to"
                    " -- Invalid\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }

        if ((registers.ai64_register > AI64_PCI_CR_SR)
            && (registers.ai64_register < AI64_PCI_CLSR_LTR_HTR_BISTR)) {
            /* Class revision id register cannot be written -- Invalid */
            DEBUGP(D_ERR,
                    "Class revision id register cannot be written to"
                    " -- Invalid\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }

        if ((registers.ai64_register > AI64_PCI_BAR3)
            && (registers.ai64_register < AI64_PCI_ERBAR)) {
            /* Subsystem id register cannot be written -- invalid */
            DEBUGP(D_ERR, "Subsystem id register cannot be written to"
                    " -- invalid\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }

        if ((registers.ai64_register > AI64_PCI_ERBAR)
            && (registers.ai64_register < AI64_PCI_ILR_IPR_MGR_MLR)) {
            /* Register 13 and 14 cannot be written */
            DEBUGP(D_ERR, "Register 13 and 14 cannot be written to"
                    " -- invalid\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }

        /* Write the value to the requested register offset */
        /*** write to the correct configuration register - drd ***/
        pci_write_config_dword(ai64_device->pdev,
                               AI64_REGISTER_INDEX(registers.
                                                   ai64_register) *
                               (sizeof(unsigned int)),
                               registers.register_value);

        IOCTL_RETURN(0);

      plx_register_write:
        DEBUGP(D_L1, "plx_register_write entry ...\n");

        if ((registers.ai64_register < AI64_PLX_LASORR)
            || (registers.ai64_register > AI64_PLX_LBRD1)) {
            /* Local config register to be written is Invalid */
            DEBUGP(D_ERR,
                    "Local config register to be written is Invalid\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }

        /* Write to the value to the register offset */
        writel(registers.register_value, (ai64_device->config_reg_address +
                                          AI64_REGISTER_INDEX(registers.
                                                              ai64_register)));
        IOCTL_RETURN(0);

    case IOCTL_AI64_ENABLE_DMA:
        DEBUGP(D_IOCTL, "IOCTL_AI64_ENABLE_DMA\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < AI64_DMA_DISABLE) || (ulval > AI64_DMA_CONTINUOUS)) {
            DEBUGP(D_ERR,
                    "IOCTL_AI64_ENABLE_DMA - invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        ai64_device->dmaState = ulval;
        IOCTL_RETURN(0);

    case IOCTL_AI64_GET_DEVICE_ERROR:
        DEBUGP(D_IOCTL, "IOCTL_AI64_GET_DEVICE_ERROR\n");
        if(arg == 0)
            ai64_device->error = AI64_DEVICE_SUCCESS;
        else
            put_user_ret(ai64_device->error, (unsigned long *) arg, (-EFAULT));
        IOCTL_RETURN(0);

    case IOCTL_AI64_INPUT_RANGE_CONFIG:
        DEBUGP(D_IOCTL, "IOCTL_AI64_INPUT_RANGE_CONFIG\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < AI64_INPUT_RANGE_2_5) || (ulval > AI64_INPUT_RANGE_10)) {
            DEBUGP(D_ERR, "IOCTL_AI64_INPUT_RANGE_CONFIG - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        regval = readlocal(ai64_device, AI64_BOARD_CTRL_REG);
        regval &= (~(AI64_BCR_RANGE_MASK));
        regval |= (ulval << AI64_BCR_RANGE_SHIFT);
        writelocal(ai64_device, regval, AI64_BOARD_CTRL_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_INPUT_MODE_CONFIG:
        DEBUGP(D_IOCTL, "IOCTL_AI64_INPUT_MODE_CONFIG\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < AI64_INPUT_SYSTEM_ANALOG_SS) || (ulval > AI64_INPUT_VREF_TEST)
            || (ulval == AI64_INPUT_RES1)) {
            DEBUGP(D_ERR, "IOCTL_AI64_INPUT_MODE_CONFIG - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }

        regval = readlocal(ai64_device, AI64_BOARD_CTRL_REG);
        regval &= (~(AI64_INPUT_MODE_MASK));
        regval |= (ulval << AI64_INPUT_MODE_SHIFT);
        writelocal(ai64_device, regval, AI64_BOARD_CTRL_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_SS_MODE_CONFIG:
        DEBUGP(D_IOCTL, "IOCTL_AI64_SS_MODE_CONFIG\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < AI64_SS_SINGLE_ENDED) || (ulval > AI64_SS_FULL_DIFFERENTIAL)) {
            DEBUGP(D_ERR, "IOCTL_AI64_SS_MODE_CONFIG - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        regval = readlocal(ai64_device, AI64_BOARD_CTRL_REG);
        regval &= (~(AI64_SS_MODE_MASK));
        regval |= (ulval << AI64_SS_MODE_SHIFT);
        writelocal(ai64_device, regval, AI64_BOARD_CTRL_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_INPUT_FORMAT_CONFIG:
        DEBUGP(D_IOCTL, "IOCTL_AI64_INPUT_FORMAT_CONFIG\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < AI64_FORMAT_TWO_COMPLEMENT)
            || (ulval > AI64_FORMAT_OFFSET_BINARY)) {
            DEBUGP(D_ERR, "IOCTL_AI64_INPUT_FORMAT_CONFIG - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        regval = readlocal(ai64_device, AI64_BOARD_CTRL_REG);
        regval &= (~(AI64_BCR_FORMAT_MASK));
        regval |= (ulval << AI64_BCR_OFFSET_BINARY_SHIFT);
        writelocal(ai64_device, regval, AI64_BOARD_CTRL_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_SET_POLAR:
        DEBUGP(D_IOCTL, "IOCTL_AI64_SET_POLAR\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval != AI64_FORMAT_UNI_POLAR)
            && (ulval != AI64_FORMAT_BI_POLAR)) {
            DEBUGP(D_ERR, "IOCTL_AI64_SET_POLAR - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        regval = readlocal(ai64_device, AI64_BOARD_CTRL_REG);
        regval &= (~(AI64_BCR_POLAR_MASK));
        if (ulval == AI64_FORMAT_UNI_POLAR)
            regval |= (AI64_BCR_UNI_POLAR << AI64_BCR_POLAR_SHIFT);
        else
            regval |= (AI64_BCR_BI_POLAR << AI64_BCR_POLAR_SHIFT);
        writelocal(ai64_device, regval, AI64_BOARD_CTRL_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_START_SINGLE_SCAN:
        DEBUGP(D_IOCTL, "IOCTL_AI64_START_SINGLE_SCAN\n");
        regval = readlocal(ai64_device, AI64_BOARD_CTRL_REG);
        regval |= AI64_BCR_INPUT_SYNCH_START;
        writelocal(ai64_device, regval, AI64_BOARD_CTRL_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_SET_SCAN_SIZE:
        DEBUGP(D_IOCTL, "IOCTL_AI64_SET_SCAN_SIZE\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < AI64_SCAN_SINGLE_CHAN) || (ulval > AI64_SCAN_64_CHAN)) {
            DEBUGP(D_ERR, "IOCTL_AI64_SET_SCAN_SIZE - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }

        ai64_device->nchans = (1 << ulval);

        regval = readlocal(ai64_device, AI64_SCAN_SYNCH_CTRL_REG);
        regval &= (~(AI64_SSCR_CHAN_SIZE_MASK));
        regval |= (ulval);
        writelocal(ai64_device, regval, AI64_SCAN_SYNCH_CTRL_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_SET_SCAN_CLOCK:
        DEBUGP(D_IOCTL, "IOCTL_AI64_SET_SCAN_CLOCK\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < AI64_INTERNAL_RATE_A) || (ulval > AI64_BCR_INPUT_SYNCH)) {
            DEBUGP(D_ERR, "IOCTL_AI64_SET_SCAN_CLOCK - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        regval = readlocal(ai64_device, AI64_SCAN_SYNCH_CTRL_REG);

        /* paranoia setting */
        ai64_device->nchans = 1 << (regval & AI64_SSCR_CHAN_SIZE_MASK);

        regval &= (~(AI64_SSCR_SCAN_CLOCK_MASK));
        regval |= (ulval << AI64_SSCR_CLOCK_SHIFT);
        writelocal(ai64_device, regval, AI64_SCAN_SYNCH_CTRL_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_DISABLE_PCI_INTERRUPTS:
        DEBUGP(D_IOCTL, "IOCTL_AI64_DISABLE_PCI_INTERRUPTS\n");
        {
            plx_irq_reg = readl(IntCntrlStat(ai64_device));
            plx_irq_reg &= ~PCI_INT_ENABLE;
            writel(plx_irq_reg, (IntCntrlStat(ai64_device)));
        }
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_ENABLE_PCI_INTERRUPTS:
        DEBUGP(D_IOCTL, "IOCTL_AI64_ENABLE_PCI_INTERRUPTS\n");
        {
            plx_irq_reg = readl(IntCntrlStat(ai64_device));
            plx_irq_reg |= PCI_INT_ENABLE;
            plx_irq_reg &= ~LOCAL_CH0_DMA_INT_ENA;  /* turn off DMA int */
            writel(plx_irq_reg, (IntCntrlStat(ai64_device)));
        }
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_SET_B_CLOCK_SOURCE:
        DEBUGP(D_IOCTL, "IOCTL_AI64_SET_B_CLOCK_SOURCE\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < AI64_MASTER_CLOCK) || (ulval > AI64_RATE_A_OUTPUT)) {
            DEBUGP(D_ERR, "IOCTL_AI64_SET_B_CLOCK_SOURCE - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        regval = readlocal(ai64_device, AI64_SCAN_SYNCH_CTRL_REG);

        /* paranoia setting */
        ai64_device->nchans = 1 << (regval & AI64_SSCR_CHAN_SIZE_MASK);

        regval &= (~(AI64_SSCR_RATE_B_CLOCK_SOURCE_MASK));
        regval |= (ulval << AI64_SSCR_RATE_B_CLOCK_SOURCE_SHIFT);
        writelocal(ai64_device, regval, AI64_SCAN_SYNCH_CTRL_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_SINGLE_CHAN_SELECT:
        DEBUGP(D_IOCTL, "IOCTL_AI64_SINGLE_CHAN_SELECT\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < 0) || (ulval > 63)) {
            DEBUGP(D_ERR, "IOCTL_AI64_SINGLE_CHAN_SELECT - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        regval = readlocal(ai64_device, AI64_SCAN_SYNCH_CTRL_REG);

        regval &= (~(AI64_SSCR_CHAN_SIZE_MASK));
        regval &= (~(AI64_SSCR_SINGLE_CHANNEL_MASK));
        regval |= (ulval << AI64_SSCR_SINGLE_CHANNEL_SHIFT);
        ai64_device->nchans = 1 << (regval & AI64_SSCR_CHAN_SIZE_MASK);
        writelocal(ai64_device, regval, AI64_SCAN_SYNCH_CTRL_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_SET_SCAN_RATE_A:
        DEBUGP(D_IOCTL, "IOCTL_AI64_SET_SCAN_RATE_A\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < 0) || (ulval > 0xFFFF)) {
            DEBUGP(D_ERR, "IOCTL_AI64_SET_SCAN_RATE_A - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        regval = readlocal(ai64_device, AI64_RATE_A_GENERATOR_REG);

        regval &= (~(AI64_RGR_NRATE_MASK));
        regval |= (ulval);
        writelocal(ai64_device, regval, AI64_RATE_A_GENERATOR_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_SET_SCAN_RATE_B:
        DEBUGP(D_IOCTL, "IOCTL_AI64_SET_SCAN_RATE_B\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < 0) || (ulval > 0xFFFF)) {
            DEBUGP(D_ERR, "IOCTL_AI64_SET_SCAN_RATE_B - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        regval = readlocal(ai64_device, AI64_RATE_B_GENERATOR_REG);

        regval &= (~(AI64_RGR_NRATE_MASK));
        regval |= (ulval);
        writelocal(ai64_device, regval, AI64_RATE_B_GENERATOR_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_GEN_DISABLE_A:
        DEBUGP(D_IOCTL, "IOCTL_AI64_GEN_DISABLE_A\n");
        regval = readlocal(ai64_device, AI64_RATE_A_GENERATOR_REG);
        regval |= AI64_RGR_GEN_DISABLE;
        writelocal(ai64_device, regval, AI64_RATE_A_GENERATOR_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_GEN_DISABLE_B:
        DEBUGP(D_IOCTL, "IOCTL_AI64_GEN_DISABLE_B\n");
        regval = readlocal(ai64_device, AI64_RATE_B_GENERATOR_REG);
        regval |= AI64_RGR_GEN_DISABLE;
        writelocal(ai64_device, regval, AI64_RATE_B_GENERATOR_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_GEN_ENABLE_A:
        DEBUGP(D_IOCTL, "IOCTL_AI64_GEN_ENABLE_A\n");
        regval = readlocal(ai64_device, AI64_RATE_A_GENERATOR_REG);
        regval &= ~(AI64_RGR_GEN_DISABLE);
        writelocal(ai64_device, regval, AI64_RATE_A_GENERATOR_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_GEN_ENABLE_B:
        DEBUGP(D_IOCTL, "IOCTL_AI64_GEN_ENABLE_B\n");
        regval = readlocal(ai64_device, AI64_RATE_B_GENERATOR_REG);
        regval &= ~(AI64_RGR_GEN_DISABLE);
        writelocal(ai64_device, regval, AI64_RATE_B_GENERATOR_REG);
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_SET_BUFFER_THRESHOLD:
        DEBUGP(D_IOCTL, "IOCTL_AI64_SET_BUFFER_THRESHOLD\n");
        if (ai64_device->read_in_progress) {
            DEBUGP(D_ERR, "Read in progress\n");
            IOCTL_RETURN(-EBUSY);
        }

        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));

		/* if this is the new 16AI64SSA/C card */
		if(ai64_device->board_type == gsc16ai64ssa_c) {
			if ((ulval <= 0) || (ulval > AI64_IDC_C_THRESHOLD_MASK)) {
				DEBUGP(D_ERR, "IOCTL_AI64_SET_BUFFER_THRESHOLD - "
                        				"invalid parameter\n");
				ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
				IOCTL_RETURN(-EINVAL);
			} 
		} else {
			/* else, old AI64SSA cards */
	        // Verify value in Range
	        if (LARGE_FIFO) {
	            if ((ulval <= 0) || (ulval / 4 > 0xFFFF)) {
	                DEBUGP(D_ERR, "IOCTL_AI64_SET_BUFFER_THRESHOLD - "
	                        "invalid parameter\n");
	                ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
	                IOCTL_RETURN(-EINVAL);
	            }
	        } else {
	            if ((ulval <= 0) || (ulval > 0xFFFF)) {
	                DEBUGP(D_ERR, "IOCTL_AI64_SET_BUFFER_THRESHOLD - "
	                        "invalid parameter\n");
	                ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
	                IOCTL_RETURN(-EINVAL);
	            }
	        }
		}

        // Get Current Reg Value
        // regval = readlocal(ai64_device, AI64_INPUT_BUFF_CTRL_REG);
        // regval &= AI64_IDC_DISABLE_BUFFER_SS;

		/* if this is the new 16AI64SSA/C card */
		if(ai64_device->board_type == gsc16ai64ssa_c) {
			regval = (ulval - 1);
			ai64_device->bufferThreshold = regval;
		} else {
			/* else, old AI64SSA cards */
	        if (LARGE_FIFO) {
	            // Large FIFO Device - Threshold value is 4x the actual value
	            if (ulval <= 0xffff) {  /* if small, do not use multiplying factor */
	                regval = ulval - 1;
	            } else {
	                if (ulval & 0x3) {  /* if not multiple of 4 */
	                    ulval += 4; /* round up to next multiple */
	                    ulval &= ~0x3;
	                }
	                regval = ((ulval / 4 - 1) | AI64_IDC_LARGE_FIFO_SS);
	                //  -1=HW Workaround ???
	            }
	            ai64_device->bufferThreshold = ulval;
	            // Large FIFO Threshold is x4 (need to adjust down)
	            DEBUGPx(D_L3, "ulval 0x%x, regval 0x%x \n",
	                    (unsigned int) ulval, (unsigned int) regval);
	        } else {
	            regval = (ulval - 1);
	            ai64_device->bufferThreshold = regval;
	        }
		}

        // ai64_device->dmaSampleSize = regval;
        ai64_device->dmaSampleSize = ulval;
        // DMA Sample Size is actual size and not adjusted
        writelocal(ai64_device, regval, AI64_INPUT_BUFF_CTRL_REG);

        REG_DUMP(D_REGS, ai64_device, "IOCTL_AI64_SET_BUFFER_THRESHOLD");

        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_CLEAR_BUFFER:
		/* if this is the new 16AI64SSA/C card */
		if(ai64_device->board_type == gsc16ai64ssa_c) {
			InputClearBuffer = AI64_IDC_C_INPUT_CLEAR_BUFFER;
		} else {
			InputClearBuffer = AI64_IDC_INPUT_CLEAR_BUFFER;
		}
        DEBUGP(D_IOCTL, "IOCTL_AI64_CLEAR_BUFFER\n");
        if (ai64_device->read_in_progress) {
            DEBUGP(D_ERR, "Read in progress\n");
            IOCTL_RETURN(-EBUSY);
        }

        regval = readlocal(ai64_device, AI64_INPUT_BUFF_CTRL_REG);
        regval |= InputClearBuffer;
        writelocal(ai64_device, regval, AI64_INPUT_BUFF_CTRL_REG);

        /// TO DO /// 
        // Poll the CLEAR BUF BIT
        // //

        //
        // Clear the Driver DMA Buffers to get rid of any old data
        //
        for (i = 0; i < ai64_device->num_dma_buffers; i++) {
            if (ai64_device->dma[i].bufferstate != DmaFree) {
                ai64_device->dma[i].bufferstate = DmaFree;
                ai64_device->dma[i].dmasamples = 0;
                ai64_device->dma[i].dmastart = 0;
                DEBUGPx(D_L1,
                        "IOCTL_AI64_CLEAR_BUFFER: Clear Buffer %d\n", i);
            }
        }   // End For i                
        ai64_device->chan0_index = 0;
        count = 5;
        while ((count--)
               && (readlocal(ai64_device, AI64_INPUT_BUFF_CTRL_REG) &
                   InputClearBuffer)) {
            wait_event_interruptible_timeout(ai64_device->ioctlwq,0,
                                           MSECS_TO_SLEEP(10));
        }
        if ((readlocal(ai64_device, AI64_INPUT_BUFF_CTRL_REG) &
             InputClearBuffer)) {
            DEBUGP(D_ERR,
                    "IOCTL_AI64_CLEAR_BUFFER: Buffer Not Cleared\n");
            REG_DUMP(D_REGS, ai64_device, "IOCTL_AI64_CLEAR_BUFFER");
            IOCTL_RETURN(-EIO);
        }

        REG_DUMP(D_REGS, ai64_device, "IOCTL_AI64_CLEAR_BUFFER");

        //
        // Clear local DMA buffer pointer so that stale data
        // is not left in the buffer
        //
        if(ai64_device->dma)
            ai64_device->dma[ai64_device->cbuffer_r].dmastart = 0;
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_ENABLE_BUFFER_SS:
		if(ai64_device->board_type == gsc16ai64ssa_c) {
        	DEBUGP(D_IOCTL, "IOCTL_AI64_ENABLE_BUFFER_SS - INVALID FOR AI64SSA/C boards\n");
        	IOCTL_RETURN(-EINVAL);
		}
        DEBUGP(D_IOCTL, "IOCTL_AI64_ENABLE_BUFFER_SS\n");
        regval = readlocal(ai64_device, AI64_INPUT_BUFF_CTRL_REG);
        regval &= ~(AI64_IDC_DISABLE_BUFFER_SS);
        writelocal(ai64_device, regval, AI64_INPUT_BUFF_CTRL_REG);
        REG_DUMP(D_REGS, ai64_device, "IOCTL_AI64_ENABLE_BUFFER_SS");

        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_DISABLE_BUFFER_SS:
		if(ai64_device->board_type == gsc16ai64ssa_c) {
        	DEBUGP(D_IOCTL, "IOCTL_AI64_DISABLE_BUFFER_SS - INVALID FOR AI64SSA/C boards\n");
        	IOCTL_RETURN(-EINVAL);
		}
        DEBUGP(D_IOCTL, "IOCTL_AI64_DISABLE_BUFFER_SS\n");
        regval = readlocal(ai64_device, AI64_INPUT_BUFF_CTRL_REG);
        regval |= AI64_IDC_DISABLE_BUFFER_SS;
        writelocal(ai64_device, regval, AI64_INPUT_BUFF_CTRL_REG);
        REG_DUMP(D_REGS, ai64_device, "IOCTL_AI64_DISABLE_BUFFER_SS");
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_DUMP_REGISTERS:
        if (ai64_device->read_in_progress) {
            DEBUGP(D_ERR, "Read in progress\n");
            IOCTL_RETURN(-EBUSY);
        }
        AI64_LOCK();
        DEBUGP(D_IOCTL, "IOCTL_AI64_DUMP_REGISTERS\n");
        REG_DUMP(D_REGS, ai64_device, "IOCTL_AI64_DUMP_REGISTERS");
        AI64_UNLOCK();
        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_GET_DRIVER_INFO:
        DEBUGP(D_IOCTL, "IOCTL_AI64_GET_DRIVER_INFO\n");
        {
            ai64_driver_info_t info;
            memset(&info, 0, sizeof(info));
            strcpy(info.version, AI64_VERSION);
            strcpy(info.built, __DATE__ ", " __TIME__);
            strcpy(info.module_name, AI64_MODULE_NAME);
            /* Copy the structure to the user */
            if(copy_to_user((void *) arg, &info, sizeof(info))) {
                IOCTL_RETURN(-EFAULT);
            }
            IOCTL_RETURN(0);
        }
        break;

	case IOCTL_AI64_MMAP_SELECT: /* New Way - use this ioctl */
        DEBUGP(D_IOCTL, "IOCTL_AI64_MMAP_SELECT ...entry\n");
        if(copy_from_user(&ai64_mmap_select, (void *) arg, sizeof(ai64_mmap_select_t))) {
            IOCTL_RETURN(-EFAULT);
        }

		switch(ai64_mmap_select.select) {
			case AI64_SELECT_GSC_MMAP:
				if(ai64_device->mem_region[LOCAL_REGION].address == 0) {
					DEBUGP (D_ERR, "Local Region Not Present\n");
					ai64_device->error = AI64_DEVICE_RESOURCE_ALLOCATION_ERROR;
            		IOCTL_RETURN(-ENOMEM);
				}	

            	bindaddr = (ai64_device->mem_region[LOCAL_REGION].address / PAGE_SIZE)
                					* PAGE_SIZE;
            	ai64_mmap_select.offset = 
						ai64_device->mem_region[LOCAL_REGION].address - bindaddr;
				ai64_mmap_select.size = ((ai64_device->mem_region[LOCAL_REGION].size +
                       + ai64_mmap_select.offset + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
			break;

			case AI64_SELECT_PLX_MMAP:
				if(ai64_device->mem_region[CONFIG_REGION].address == 0) {
					DEBUGP (D_ERR, "Config Region Not Present\n");
					ai64_device->error = AI64_DEVICE_RESOURCE_ALLOCATION_ERROR;
            		IOCTL_RETURN(-ENOMEM);
				}	
            	bindaddr = (ai64_device->mem_region[CONFIG_REGION].address / PAGE_SIZE)
                					* PAGE_SIZE;
            	ai64_mmap_select.offset = 
						ai64_device->mem_region[CONFIG_REGION].address - bindaddr;

				ai64_mmap_select.size = ((ai64_device->mem_region[CONFIG_REGION].size +
                       + ai64_mmap_select.offset + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
			break;

			case AI64_SELECT_PHYS_MEM_MMAP:
            	ai64_mmap_select.offset = 0;
            	ai64_mmap_select.size = 0;
			break;

			default:
            	ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            	DEBUGPx(D_ERR, "invalid select: mmap_reg_select=x%lx\n",
                    mmap_reg_select);
            	IOCTL_RETURN(-EINVAL);
			break;
		}

		/* tell driver what user has selected */
        ai64_device->mmap_reg_select = ai64_mmap_select.select;

        if(copy_to_user((void *) arg, &ai64_mmap_select, sizeof(ai64_mmap_select_t))) {
            IOCTL_RETURN(-EFAULT);
        }

        break;

    case IOCTL_AI64_MMAP_GET_OFFSET: /* 01/07/2014 - ####OBSOLETE#### */
        DEBUGP(D_IOCTL, "IOCTL_AI64_MMAP_GET_OFFSET ...entry\n");

        /* Get the argument from the user */
        get_user(mmap_reg_select, (unsigned long *) arg);

        /* Check if the argument passed is valid */
        if ((mmap_reg_select != AI64_SELECT_GSC_MMAP) &&
            (mmap_reg_select != AI64_SELECT_PLX_MMAP) &&
            (mmap_reg_select != AI64_SELECT_PHYS_MEM_MMAP)) {
            /* Invalid argument */
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            DEBUGPx(D_ERR, "invalid select: mmap_reg_select=x%lx\n",
                    mmap_reg_select);
            IOCTL_RETURN(-EINVAL);
        }

        ai64_device->mmap_reg_select = mmap_reg_select;

        if (ai64_device->mmap_reg_select == AI64_SELECT_PHYS_MEM_MMAP) {
            bindoffset = 0;
        } else {
            region = (mmap_reg_select == AI64_SELECT_GSC_MMAP) ?
                LOCAL_REGION : CONFIG_REGION;

            bindaddr =
                (ai64_device->mem_region[region].address / PAGE_SIZE)
                * PAGE_SIZE;
            bindoffset =
                ai64_device->mem_region[region].address - bindaddr;
        }
        if(copy_to_user((void *) arg, &bindoffset, sizeof(unsigned long))) {
            IOCTL_RETURN(-EFAULT);
        }

        IOCTL_RETURN(0);
        break;

    case IOCTL_AI64_ENABLE_TEST_PATTERN:
        DEBUGP(D_IOCTL, "IOCTL_AI64_ENABLE_TEST_PATTERN\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < 0) || (ulval > 1)) {
            DEBUGP(D_ERR, "IOCTL_AI64_ENABLE_TEST_PATTERN - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        ai64_device->enable_test_pattern = (unsigned short) ulval;
        ai64_device->Test_Pattern = 0;
        IOCTL_RETURN(0);

    case IOCTL_AI64_RETURN_16BIT_SAMPLES:
        DEBUGP(D_IOCTL, "IOCTL_AI64_16BIT_SAMPLES\n");
        if (ai64_device->read_in_progress) {
            DEBUGP(D_ERR, "Read in progress\n");
            IOCTL_RETURN(-EBUSY);
        }

        AI64_LOCK();

        if(ai64_device->num_dma_buffers == 0) {
            DEBUGP(D_ERR, "IOCTL_AI64_RETURN_16BIT_SAMPLES - "
                    "No DMA buffers specified\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            AI64_UNLOCK();
            IOCTL_RETURN(-EINVAL);
        }

        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < 0) || (ulval > 1)) {
            DEBUGP(D_ERR, "IOCTL_AI64_RETURN_16BIT_SAMPLES - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            AI64_UNLOCK();
            IOCTL_RETURN(-EINVAL);
        }

        /* if we are changing sample size */
        if (ai64_device->return_16bit_samples != (unsigned short) ulval) {
            /* if buffer in use, dump data and free buffer */
            if (ai64_device->dma[ai64_device->cbuffer_r].bufferstate ==
                DmaInUse) {
                ai64_device->dma[ai64_device->cbuffer_r].bufferstate =
                    DmaFree;
                ai64_device->dma[ai64_device->cbuffer_r].dmasamples = 0;    // Drain the dma buffer 
                ai64_device->dma[ai64_device->cbuffer_r].dmastart = 0;  // Reset the ptr/size to 0
                ai64_device->cbuffer_r =
                    (++ai64_device->cbuffer_r %
                     ai64_device->num_dma_buffers);
                ai64_hwm_check(ai64_device);    /* Check for HWM */
            }

            ai64_device->return_16bit_samples = (unsigned short) ulval;
        }

        if (ai64_device->return_16bit_samples)  /* if 16bit samples */
            ai64_device->bytes_per_sample = sizeof(ushort); /* 16bit samples */
        else
            ai64_device->bytes_per_sample = sizeof(u32);    /* 32bit samples */

        AI64_UNLOCK();  // Release Lock
        IOCTL_RETURN(0);

    case IOCTL_AI64_VALIDATE_CHAN_0:
        DEBUGP(D_IOCTL, "IOCTL_AI64_VALIDATE_CHAN_0\n");
        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval < 0) || (ulval > 1)) {
            DEBUGP(D_ERR, "IOCTL_AI64_VALIDATE_CHAN_0 - "
                    "invalid parameter\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }
        ai64_device->validate_chan_0 = (unsigned short) ulval;
        IOCTL_RETURN(0);

    case IOCTL_AI64_GET_BUFFER_HWM:
        DEBUGP(D_IOCTL, "IOCTL_AI64_GET_BUFFER_HWM\n");
        /* If pointer to ai64_hwm struct is supplied, return HWM */
        if (arg) {
            buf_hwm.total_buffers = ai64_device->num_dma_buffers;
            buf_hwm.buffers_inuse_hwm = ai64_device->buf_inuse_hwm;
            if(copy_to_user((void *) arg, &buf_hwm, sizeof(buf_hwm))) {
                IOCTL_RETURN(-EFAULT);
            }
        } else {    /* clear HWM */
            ai64_device->buf_inuse_hwm = 0; /* clear buffer hwm */
        }
        IOCTL_RETURN(0);

    case IOCTL_AI64_CTRL_AUX_SYNC_IO:
        DEBUGP(D_IOCTL, "IOCTL_AI64_CTRL_AUX_SYNC_IO\n");
        if(copy_from_user(&aux_sync_io, (void *) arg, sizeof(aux_sync_io))) {
            IOCTL_RETURN(-EFAULT);
        }

        if ((aux_sync_io.write != 0) && (aux_sync_io.write != 1)) {
            DEBUGPx(D_ERR, "invalid select: function=x%d\n",
                    aux_sync_io.write);
            IOCTL_RETURN(-EINVAL);
        }

        if (aux_sync_io.write == 0) {   /* return AUX SYNC info */
            memset(&aux_sync_io, 0, sizeof(aux_sync_io));
            regval = readlocal(ai64_device, AI64_AUX_SYNC_IO_CTRL);
            DEBUGPx(D_L1, "AI64_AUX_SYNC_IO_CTRL=0x%lx\n", regval);
            aux_sync_io.aux_0 = (regval & AI64_AUX_0_CTRL_MASK) >>
                AI64_AUX_0_CTRL_SHIFT;
            aux_sync_io.aux_1 = (regval & AI64_AUX_1_CTRL_MASK) >>
                AI64_AUX_1_CTRL_SHIFT;
            aux_sync_io.aux_2 = (regval & AI64_AUX_2_CTRL_MASK) >>
                AI64_AUX_2_CTRL_SHIFT;
            aux_sync_io.aux_3 = (regval & AI64_AUX_3_CTRL_MASK) >>
                AI64_AUX_3_CTRL_SHIFT;
            aux_sync_io.invert_inputs = (regval & AI64_AUX_INVERT_INP_MASK) >>
                AI64_AUX_INVERT_INP_SHIFT;
            aux_sync_io.invert_outputs = (regval & AI64_AUX_INVERT_OUT_MASK) >>
                AI64_AUX_INVERT_OUT_SHIFT;
            aux_sync_io.noise_suppress = (regval & AI64_AUX_NOISE_SUP_MASK) >>
                AI64_AUX_NOISE_SUP_SHIFT;
            /* Copy the structure to the user */
            if(copy_to_user((void *) arg, &aux_sync_io, sizeof(aux_sync_io))) {
                IOCTL_RETURN(-EFAULT);
            }
            IOCTL_RETURN(0);
        }

        if (aux_sync_io.write == 1) {   /* set AUX SYNC info */
            regval = 0;
            regval |= ((aux_sync_io.aux_0 << AI64_AUX_0_CTRL_SHIFT) &
                       AI64_AUX_0_CTRL_MASK);
            regval |= ((aux_sync_io.aux_1 << AI64_AUX_1_CTRL_SHIFT) &
                       AI64_AUX_1_CTRL_MASK);
            regval |= ((aux_sync_io.aux_2 << AI64_AUX_2_CTRL_SHIFT) &
                       AI64_AUX_2_CTRL_MASK);
            regval |= ((aux_sync_io.aux_3 << AI64_AUX_3_CTRL_SHIFT) &
                       AI64_AUX_3_CTRL_MASK);
            regval |=
                ((aux_sync_io.
                  invert_inputs << AI64_AUX_INVERT_INP_SHIFT) &
                 AI64_AUX_INVERT_INP_MASK);
            regval |=
                ((aux_sync_io.
                  invert_outputs << AI64_AUX_INVERT_OUT_SHIFT) &
                 AI64_AUX_INVERT_OUT_MASK);
            regval |=
                ((aux_sync_io.
                  noise_suppress << AI64_AUX_NOISE_SUP_SHIFT) &
                 AI64_AUX_NOISE_SUP_MASK);
            writelocal(ai64_device, regval, AI64_AUX_SYNC_IO_CTRL);
            DEBUGPx(D_L1, "AI64_AUX_SYNC_IO_CTRL=0x%lx\n", regval);
            IOCTL_RETURN(0);
        }

        IOCTL_RETURN(0);

    case IOCTL_AI64_GET_PHYSICAL_MEMORY:
        DEBUGP(D_IOCTL, "IOCTL_AI64_GET_PHYSICAL_MEMORY\n");
        /* If pointer to ai64_phys_mem_t struct is supplied, return physical DMA memory */
        if (arg) {
            if(copy_from_user(&ai64_phys_mem, (void *) arg,
                           sizeof(ai64_phys_mem_t))) {
                IOCTL_RETURN(-EFAULT);
            }
            /* if no physical memory allocated */
            if (ai64_device->phys_mem == 0) {
                ai64_device->error = AI64_DEVICE_RESOURCE_ALLOCATION_ERROR;
                DEBUGP(D_ERR,
                        "Physical memory not allocated. Do mmap()\n");
                IOCTL_RETURN(-ENOMEM);
            }

            ai64_phys_mem.phys_mem = ai64_device->phys_mem;
            ai64_phys_mem.phys_mem_size = ai64_device->phys_mem_size;
            if(copy_to_user((void *) arg, &ai64_phys_mem,
                         sizeof(ai64_phys_mem_t))) {
                IOCTL_RETURN(-EFAULT);
            }
            IOCTL_RETURN(0);
        } else {
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            DEBUGP(D_ERR,
                    "Parameter must be pointer to ai64_phys_mem_t typedef\n");
            IOCTL_RETURN(-EINVAL);
        }

        IOCTL_RETURN(0);

    case IOCTL_AI64_ALLOCATE_DMA_BUFFERS:
        DEBUGP(D_IOCTL, "IOCTL_AI64_ALLOCATE_DMA_BUFFERS\n");
        if (ai64_device->use_resmem) {
            DEBUGP(D_ERR,
                    "Cannot Allocate - Memory already reserved\n");
            ai64_device->error = AI64_DEVICE_RESOURCE_IN_USE;
            IOCTL_RETURN(-EADDRINUSE);
        }

        get_user_ret(ulval, (unsigned long *) arg, (-EFAULT));
        if ((ulval <= 0)) {
                DEBUGP(D_ERR, "IOCTL_AI64_ALLOCATE_DMA_MEMORY: - "
                        "invalid parameter\n");
                ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
                IOCTL_RETURN(-EINVAL);
        }

        if(ai64_device->num_dma_buffers) {
            DEBUGP(D_ERR,
                    "Cannot Allocate - Memory already allocated\n");
            ai64_device->error = AI64_DEVICE_RESOURCE_IN_USE;
            IOCTL_RETURN(-EADDRINUSE);
        }

        ai64_device->num_dma_buffers = ulval;

        /* If DMA buffer structure allocated */
        if(ai64_device->dma) {
            kfree(ai64_device->dma);
            ai64_device->dma = NULL;
        }

        /* Allocate DMA buffer structure */
        ai64_device->dma =
            (struct ai_dma *) kmalloc(ai64_device->num_dma_buffers *
                                          sizeof(struct ai_dma),
                                          GFP_KERNEL);
        if (ai64_device->dma == NULL) {
            ERRP("Unable to allocate memory\n");
            ai64_device->error = AI64_DEVICE_RESOURCE_ALLOCATION_ERROR;
            ai64_device->num_dma_buffers = 0;
            IOCTL_RETURN(-ENOMEM);
        }
        memset(ai64_device->dma, 0, ai64_device->num_dma_buffers * 
                        sizeof(struct ai_dma));  /* zero structure */

        for (i = 0; i < ai64_device->num_dma_buffers; i++) {
            DEBUGPx(D_BUFF,
                    "\n*******: ALLOCATE DMA BUFFER MEMORY "
                    "(#%d Buffer), DMA_ORDER (0x%x)  *******\n",
                    i, DMA_ORDER);
            DEBUGPx(D_BUFF,
                    "*******: DMA_SIZE (0x%x) : DMA_SAMPLES "
                    "(0x%x) PAGE_SIZE (0x%x) *******\n",
                    (unsigned int) DMA_SIZE,
                    (unsigned int) DMA_SAMPLES,
                    (unsigned int) PAGE_SIZE);

            /* __get_free_pages used to allocated above 16Mb range 
             * for PCI devices. Using __get_dma_pages will limit 
             * allocated pages to within 16MB. 
             */
            if (!(ai64_device->dma[i].dmabufferpool =
                  (u32 *) __get_free_pages(GFP_KERNEL | GFP_DMA32,
                                           DMA_ORDER))) {
                ERRPx("can not allocate DMA pages: "
                      "expected=%d allocated=%d\n",
                      ai64_device->num_dma_buffers, i);
                        
                DEBUGPx(D_BUFF,
                        "\n\n\n\n*******: RELEASE ALLOCATED MEMORY "
                        "(%d Buffers)  *******\n", i);
				for(j=0; j < i; j++) {
                    free_pages((uintptr_t) ai64_device->dma[j].
                               dmabufferpool, DMA_ORDER);
                    ai64_device->dma[j].dmabufferpool = 0;
                }

                ai64_device->error = AI64_DEVICE_RESOURCE_ALLOCATION_ERROR;
                ai64_device->num_dma_buffers = 0;
                IOCTL_RETURN(-ENOMEM);
            }

            DEBUGPx(D_BUFF, "### Allocate: %d : 0x%p\n",
                    i, ai64_device->dma[i].dmabufferpool);
            ai64_device->dma[i].dmaoffset =
                virt_to_phys((void *) ai64_device->dma[i].
                             dmabufferpool);

            /* If DMA address beyond 4GB, error out */
            if(ai64_device->dma[i].dmaoffset > 0xFFFFFFFFL) {
                printk (KERN_INFO AI64_DEVICE_NAME 
                        ": memory allocation failed: "
                        "kernel returned memory above 4GB (0x%lx)\n",   
                        ai64_device->dma[i].dmaoffset);

                for(j=0; j <= i; j++) {
                    free_pages((uintptr_t) ai64_device->dma[j].
                               dmabufferpool, DMA_ORDER);
                    ai64_device->dma[j].dmabufferpool = 0;
                }
                ai64_device->error = AI64_DEVICE_RESOURCE_ALLOCATION_ERROR;
                ai64_device->num_dma_buffers = 0;
                IOCTL_RETURN(-ENOMEM);
            }

            ai64_device->dma[i].bufferstate = DmaFree;
            ai64_device->dma[i].dmasamples = 0;
            ai64_device->dma[i].dmastart = 0;
            DEBUGPx(D_BUFF,
                    "DMA virt=0x%p, phys=0x%lx, status = %d \n",
                    ai64_device->dma[i].dmabufferpool,
                    ai64_device->dma[i].dmaoffset,
                    ai64_device->dma[i].bufferstate);

        }   // End For i

        // Fill buffer with pattern
        for (i = 0; i < ai64_device->num_dma_buffers; i++) {
            ai64_device->dmabuffer = ai64_device->dma[i].dmabufferpool;
            for (j = 0; j < DMA_SAMPLES; j++) {
                ai64_device->dmabuffer[j] = 0xDEADBEEF;
            }
        }

        ai64_device->read_in_progress = 0;
        ai64_device->dmaState = AI64_DMA_DISABLE;
        ai64_device->cbuffer_r = 0;
        ai64_device->cbuffer_w = 0;
        ai64_device->buf_inuse_hwm = 0;
        ai64_device->dmainprogress = NO_DMA_INPROGRESS;
        ai64_device->wakeup_dma_pending = FALSE;
        ai64_device->wakeup_lohi_pending = FALSE;
        ai64_device->hw_init_during_read = 0;
        ai64_device->dma_abort_request = 0;
        ai64_device->read_to_issue_dma = 0;
        ai64_device->Test_Pattern = 0;  /* for debug only */
        ai64_device->chan0_index = 0;
        ai64_device->ReadCount = 0;
        ai64_device->WriteCount = 0;
        ai64_device->timestamp_read_hwm = 0;
        ai64_device->timestamp_btw_read_hwm = 0;

        IOCTL_RETURN(0);

    case IOCTL_AI64_GET_NUM_DMA_BUFFERS:
        {
            ai64_num_dma_bufs_t bufs;
            DEBUGP(D_IOCTL, "IOCTL_AI64_GET_NUM_DMA_BUFFERS\n");
            bufs.num_dma_buffers = ai64_device->num_dma_buffers;
            bufs.use_resmem = ai64_device->use_resmem;
            /* Copy the structure to the user */
            if(copy_to_user((void *) arg, &bufs, sizeof(bufs))) {
                IOCTL_RETURN(-EFAULT);
            }
            IOCTL_RETURN(0);
        }
        break;

    case IOCTL_AI64_REMOVE_DMA_BUFFERS:
        DEBUGP(D_IOCTL, "IOCTL_AI64_REMOVE_DMA_BUFFERS\n");
        if (ai64_device->use_resmem) {
            DEBUGP(D_ERR,
                    "Cannot Remove - Memory is reserved\n");
            ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
            IOCTL_RETURN(-EINVAL);
        }

        if(ai64_device->num_dma_buffers) {
            ai64_abort_any_dma_inprogress(ai64_device);
            for (i = 0; i < ai64_device->num_dma_buffers; i++) {
                free_pages((uintptr_t) ai64_device->dma[i].dmabufferpool, 
                                                                DMA_ORDER);
                    ai64_device->dma[i].dmabufferpool = 0;
            }
            ai64_device->num_dma_buffers = 0;
        }
                
        IOCTL_RETURN(0);

    case IOCTL_AI64_ADD_IRQ:
        DEBUGP(D_IOCTL, "IOCTL_AI64_ADD_IRQ\n");
        if (ai64_device->irq_added == FALSE) {
            if(ai64_device->irqlevel >= 0) {
                if (request_irq
                    (ai64_device->irqlevel, device_interrupt, 
//                     SA_INTERRUPT | SA_SHIRQ, AI64_DEVICE_NAME, 
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
                       IRQF_DISABLED | IRQF_SHARED, AI64_DEVICE_NAME, 
#else
                       IRQF_SHARED, AI64_DEVICE_NAME, 
#endif
                                                         ai64_device) < 0) {
                    DEBUGPx(D_ERR, "can not get interrupt %d\n",
                            ai64_device->irqlevel);
                    IOCTL_RETURN(-EBUSY);
                }
                ai64_device->irq_added = TRUE;
            }

            /* enable interrupts */
            plx_irq_reg = readl(IntCntrlStat(ai64_device));
            plx_irq_reg |= PCI_INT_ENABLE;
            plx_irq_reg &= ~LOCAL_CH0_DMA_INT_ENA;  /* turn off DMA int */
            writel(plx_irq_reg, (IntCntrlStat(ai64_device)));
        }

        IOCTL_RETURN(0);

    case IOCTL_AI64_REMOVE_IRQ:
        DEBUGP(D_IOCTL, "IOCTL_AI64_REMOVE_IRQ\n");
        if (ai64_device->irq_added == TRUE) {
            /* remove interrupts */
            plx_irq_reg = readl(IntCntrlStat(ai64_device));
            plx_irq_reg &= ~PCI_INT_ENABLE;
            writel(plx_irq_reg, (IntCntrlStat(ai64_device)));

            free_irq(ai64_device->irqlevel, ai64_device);
            ai64_device->irq_added = FALSE;
        }
        IOCTL_RETURN(0);

    default:
        DEBUGPx(D_ERR, "unexpected IOCTL 0x%x\n", num);
        IOCTL_RETURN(-EINVAL);
    }
        
    IOCTL_RETURN(0);
}

/********************************************************************
 *                                                                  *
 *      Configure memory regions                                    *
 *                                                                  *
 ********************************************************************/
int
ai64_configure_mem_regions(struct ai_board *ai64_device,
                           struct pci_dev *pdev, int which, u32 * address,
                           u32 * size, u32 * flags)
{
    DEBUGP_ENTER;
    /*** First get the requested memory region ***/
    *address = 0;
    *address = pci_resource_start(pdev, which);
    *size = pci_resource_end(pdev, which) - *address + 1;
    DEBUGPx(D_L2, "PCI_BASE_ADDRESS_%d: address = 0x%x, size = 0x%x\n",
            which, *address, *size);

    /*** Next, request the memory regions so that they are marked busy ***/

    *flags = pdev->resource[which].flags;

    /* if no BAR present, skip it */
    if(*address == 0) {
        DEBUGPx(D_L2, "Base address ZERO: Skipping BAR%d\n",which);
        DEBUGP_EXIT;
        return (0);
    }

    /* I/O Region */
    if (pdev->resource[which].flags & IORESOURCE_IO) {
        DEBUGPx(D_L2, "Device %x Vendor %x:  region %d is I/O mapped\n",
                pdev->device, pdev->vendor, which);

        if (!request_region(*address, *size, AI64_MODULE_NAME)) {
            DEBUGPx(D_L2, "Device %x Vendor %x:  I/O region %d is busy\n",
                    pdev->device, pdev->vendor, which);
            *address = 0;   /* do not release it */
            DEBUGP(D_ERR, "request_region(IO REGION) failed\n");
            DEBUGP_EXIT;
            return (1);
        }
    } else {    /* Memory Region */
        DEBUGPx(D_L2, "Device %x Vendor %x:  region %d is memory mapped\n",
                pdev->device, pdev->vendor, which);

        if (!request_mem_region(*address, *size, AI64_MODULE_NAME)) {
            DEBUGPx(D_L2,
                    "Device %x Vendor %x:  memory region %d is busy\n",
                    pdev->device, pdev->vendor, which);
            *address = 0;   /* do not release it */
            DEBUGP(D_ERR, "request_region(MEMORY REGION) failed\n");
            DEBUGP_EXIT;
            return (1);
        }
    }
    DEBUGP_EXIT;
    return (0);
}


/********************************************************************
 *                                                                  *
 *      De-configure memory regions                                 *
 *                                                                  *
 ********************************************************************/
void
ai64_deconfigure_mem_regions(struct ai_board *ai64_device, int which,
                             u32 address, u32 size, u32 flags)
{
    DEBUGP_ENTER;
    DEBUGPx(D_L2, "ai64_deconfigure_mem_regions = 0x%x "
            "size=0x%x flags=0x%x\n", address, size, flags);

    /* if no address to deconfigure, simply return */
    if (!address) {
        DEBUGP_EXIT;
        return;
    }

    /* I/O Region */
    if (flags & IORESOURCE_IO) {
        DEBUGPx(D_L2, "Address %x Size %x:  Deconfigure I/O region %d\n",
                address, size, which);

        release_region(address, size);

    } else {    /* Memory Region */

        DEBUGPx(D_L2,
                "Address %x Size %x:  Deconfigure memory region %d\n",
                address, size, which);

        release_mem_region(address, size);
    }
    DEBUGP_EXIT;
}

/********************************************************************
 *                                                                  *
 *      mmap() support                                              *
 *                                                                  *
 ********************************************************************/
static int device_mmap(struct file *fp, struct vm_area_struct *vma)
{
    struct ai_board *ai64_device = 0;
    u32 size, round_size;
    unsigned long page;
    int ret = 0;
    unsigned int offset = vma->vm_pgoff << PAGE_SHIFT;

    /* Get the device structure from the private_data field */
    ai64_device = (struct ai_board *) fp->private_data;
    DEBUGP_ENTER;
    size = vma->vm_end - vma->vm_start;

    DEBUGPx(D_L2, "mmap(): physical local register: "
            "address=0x%x size=0x%x\n",
            ai64_device->mem_region[LOCAL_REGION].address,
            ai64_device->mem_region[LOCAL_REGION].size);
    DEBUGPx(D_L2, "mmap(): physical config register: "
            "address=0x%x size=0x%x\n",
            ai64_device->mem_region[CONFIG_REGION].address,
            ai64_device->mem_region[CONFIG_REGION].size);

    DEBUGPx(D_L2, "mmap(): start=0x%x end=0x%x offset=0x%x\n",
            (int) vma->vm_start, (int) vma->vm_end, offset);

    /* offset must be zero */
    if (offset) {
        DEBUGPx(D_L2, "mmap(): Invalid offset 0x%x. Offset must be zero\n",
                offset);
        ret = -EINVAL;
    }

    /* Use mmap_reg_select to distinguish between Control/DataGSC  register 
     *  and PLX register */
    switch ((int) ai64_device->mmap_reg_select) {
    case AI64_SELECT_GSC_MMAP: /* Control/Data - AI64 (GSC) Registers */
        round_size = ((ai64_device->mem_region[LOCAL_REGION].size +
                       PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

        DEBUGPx(D_L2, "Input Size = 0x%x Local Page "
                "Rounded Size = 0x%x\n", (u32) size, (u32) round_size);

        if (size > round_size) {
            ret = -EINVAL;
            break;
        }

        page = ai64_device->mem_region[LOCAL_REGION].address;
        DEBUGPx(D_L2, "mmap(): GSC vm_start=0x%x page=0x%x\n",
                (int) vma->vm_start, (int) page);

	    vma->vm_flags |= VM_IO; /* NEEDED IF USER DOES AN mlockall() */

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,9)
        if (remap_page_range
            (vma,vma->vm_start, page, size, vma->vm_page_prot)) {
            DEBUGPx(D_L2,
                    "mmap(): remap_page_range() failed - vm_start=0x%x "
                    "page=0x%x, size=0x%x\n", (int) vma->vm_start,
                    (int) page, (int) size);
            ret = -EAGAIN;
        }
#else
        if (remap_pfn_range
            (vma,vma->vm_start,(page >> PAGE_SHIFT), size, vma->vm_page_prot)) {
            DEBUGPx(D_L2,
                    "mmap(): remap_pfn_range() failed - vm_start=0x%x "
                    "page=0x%x, size=0x%x\n", (int) vma->vm_start,
                    (int) page, (int) size);
            ret = -EAGAIN;
        }
#endif
        break;

    case AI64_SELECT_PLX_MMAP: /* PLX Registers */
        round_size = ((ai64_device->mem_region[CONFIG_REGION].size +
                       PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
        DEBUGPx(D_L2, "Input Size = 0x%x Local Page Rounded "
                "Size = 0x%x\n", (u32) size, (u32) round_size);
        if (size > round_size) {
            ret = -EINVAL;
            break;
        }

        page = ai64_device->mem_region[CONFIG_REGION].address;

        DEBUGPx(D_L2, "mmap(): PLX vm_start=0x%x page=0x%x\n",
                (int) vma->vm_start, (int) page);

	    vma->vm_flags |= VM_IO; /* NEEDED IF USER DOES AN mlockall() */

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,9)
        if (remap_page_range
            (vma,vma->vm_start, page, size, vma->vm_page_prot)) {
            DEBUGPx(D_L2,
                    "mmap(): remap_page_range() failed - vm_start=0x%x "
                    "page=0x%x, size=0x%x\n", (int) vma->vm_start,
                    (int) page, (int) size);
            ret = -EAGAIN;
        }
#else
        if (remap_pfn_range
            (vma,vma->vm_start,(page >> PAGE_SHIFT), size, vma->vm_page_prot)) {
            DEBUGPx(D_L2,
                    "mmap(): remap_pfn_range() failed - vm_start=0x%x "
                    "page=0x%x, size=0x%x\n", (int) vma->vm_start,
                    (int) page, (int) size);
            ret = -EAGAIN;
        }
#endif
        break;


    case AI64_SELECT_PHYS_MEM_MMAP:
        vma->vm_ops = &vm_ops;

        /* if allocation of physical memory failed, error return */
        if ((ret = ai64_allocate_physical_memory(ai64_device, size))) {
            DEBUGP_EXIT;
            return ret;
        }

        page = (unsigned long)ai64_device->phys_mem;

        DEBUGPx(D_L2, "mmap(): GSC vm_start=0x%x page=0x%x\n",
                (int) vma->vm_start, (int) page);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,9)
        if (remap_page_range
            (vma, vma->vm_start, page, size, vma->vm_page_prot)) {
            DEBUGPx(D_L2,
                    "mmap(): remap_page_range() failed - vm_start=0x%x "
                    "page=0x%x, size=0x%x\n", (int) vma->vm_start,
                    (int) page, (int) size);
            ai64_free_physical_memory(ai64_device);
            ret = -EAGAIN;
        }
#else
        if (remap_pfn_range
            (vma, vma->vm_start,(page>>PAGE_SHIFT), size, vma->vm_page_prot)) {
            DEBUGPx(D_L2,
                    "mmap(): remap_pfn_range() failed - vm_start=0x%x "
                    "page=0x%x, size=0x%x\n", (int) vma->vm_start,
                    (int) page, (int) size);
            ai64_free_physical_memory(ai64_device);
            ret = -EAGAIN;
        }
#endif

        DEBUGP_EXIT;
        return ret;
        break;

    default:
        DEBUGPx(D_ERR, "Invalid parameter %ld\n",
                ai64_device->mmap_reg_select);
        ret = -EINVAL;
        break;
    }

    DEBUGP_EXIT;
    return ret;
}

#define	SEARCH_INCREMENT	(128*1024*1024) /* 128 MB */
#define	MATCH_PATTERN		0x12345678

/**************************************************************
 *            ai64_locate_memory_range()                      *
 **************************************************************/
void ai64_locate_memory_range(void **kernel_top, void **processor_physmem)
{
#ifdef AI64_DEBUG   /* only used for debug message */
    struct ai_board *ai64_device = 0;    /* dummy for use with DEBUGPx*/
#endif                          /* AI64_DEBUG */
    u32 read_pattern;
    void *kp_next, *kp;
    unsigned int last_word_offset;
    volatile u32 *vp;
    uintptr_t Uptr;
        
    DEBUGP_ENTER;

    /*** First calculate the top of kernel memory ***/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
    kp = (void *) (get_num_physpages() << PAGE_SHIFT);    /* kernel top of memory */
#else
    kp = (void *) (num_physpages << PAGE_SHIFT);    /* kernel top of memory */
#endif

    /* round up to DMA_SIZE */
    /* to work on 64 bit architecture */
    Uptr = (uintptr_t) kp;
    Uptr = (Uptr + (DMA_SIZE - 1)) & ~(DMA_SIZE - 1);
    *kernel_top = (void *) Uptr;

    // *kernel_top = (kp + (DMA_SIZE-1)) & ~(DMA_SIZE-1);

    /*** Next, calculate the real memory. Currently I do not know if the 
	 *** kernel has an external variable(s) providing this information.
	 ***/
    last_word_offset = (DMA_SIZE / sizeof(unsigned int)) - 1;

    Uptr = (uintptr_t) kp;
    Uptr = (Uptr + (SEARCH_INCREMENT - 1)) & ~(SEARCH_INCREMENT - 1);
    /* round to increment */
    kp = (void *) Uptr;

    // kp = (kp + (SEARCH_INCREMENT-1)) & ~(SEARCH_INCREMENT-1);    
    /* round to increment */

    while (1) { /* Search for real memory presence */
        kp_next = kp + SEARCH_INCREMENT;    /* get next increment */

        vp = ioremap((uintptr_t) kp_next - DMA_SIZE, DMA_SIZE);
        /* map last buffer */
        DEBUGPx(D_L2, "Physical=0x%p, virtual=0x%p\n",
                (kp_next - DMA_SIZE), vp);
        if (vp) {
            vp[last_word_offset] = MATCH_PATTERN;
            read_pattern = vp[last_word_offset];
            iounmap((u32 *) vp);
            if (read_pattern != MATCH_PATTERN)
                break;
        }
        kp = kp_next;   /* get next increment block */
    }

    *processor_physmem = kp;
        
    DEBUGP_EXIT;
}

/**************************************************************
 *                     ai64_check_args()                      *
 **************************************************************/
int ai64_check_args(void *kernel_top, void *processor_physmem, int board_count)
{
    struct ai_board *ai64_device = 0;
    int i, error = 1, total_nbufs;
    uintptr_t request_end;

    DEBUGP_ENTER;
    /* if -1 for resmem size, then use top of kernel space */
    if (RESMEM_START == -1)
        RESMEM_START = (uintptr_t) kernel_top;

    /* check for start address */
    if ((uintptr_t) RESMEM_START < (uintptr_t) kernel_top) {
        ERRPx("specified resmem start (0x%x) cannot be less than "
              "kernel top (0x%p)\n", RESMEM_START, kernel_top);
        DEBUGP_EXIT;
        return (error);
    }

    if ((uint) RESMEM_START > (uintptr_t) processor_physmem) {
        ERRPx("specified resmem start (0x%x) cannot be greater than "
              "physical memory (0x%p)\n", RESMEM_START, processor_physmem);
        DEBUGP_EXIT;
        return (error);
    }

    if (RESMEM_SIZE == -1)
        RESMEM_SIZE = (uintptr_t) processor_physmem - RESMEM_START;

    /* check to see if we do not go past physical memory */
    request_end = (uintptr_t) RESMEM_START + (uintptr_t) RESMEM_SIZE;
    if (request_end > (uintptr_t) processor_physmem) {
        ERRPx
            ("specified resmem start/size (0x%x/0x%x) exceeds physical memory"
             " (0x%p)\n", RESMEM_START, RESMEM_SIZE, processor_physmem);
        DEBUGP_EXIT;
        return (error);
    }

    /***********************************************************
     * determine total number of buffers requested
     *    if nbufs=  0, no buffers to be allocated,
     *    if nbufs=  #, # of buffers to be allocated or reserved,
     *    if nbufs= -#, allocate (not reserve) number of buffers
     */
    total_nbufs = 0;
    for (i = 0; i < board_count; i++) {
        /* if no buffers, or buffers to be allocated, skip count */
        if((*nbufs[i]) <= 0 )
            continue;

#if 0
        /* if any buf size is specified and less that 2, error out */
        if (*nbufs[i] < 2) {
            ERRPx("b%d=%d number of buffers must not be less than 2\n",
                  i, *nbufs[i]);
            DEBUGP_EXIT;
            return (error);
        }
#endif

        total_nbufs += *nbufs[i];
    }

    /* if no buffer sizes have been specified, return */
    if (!total_nbufs) {
        RESMEM_START = RESMEM_SIZE = 0;
        DEBUGP_EXIT;
        return (0);
    }

    /* set resmem size to multiple of DMA_SIZE */
    RESMEM_SIZE = RESMEM_SIZE & ~(DMA_SIZE - 1);

    if ((unsigned int) (total_nbufs * DMA_SIZE) >
        (unsigned int) RESMEM_SIZE) {
        ERRPx
            ("total no. requested buffers=%d is greater than available=%d for "
             "resmem size 0x%x\n", total_nbufs,
             (int) (RESMEM_SIZE / DMA_SIZE), RESMEM_SIZE);
        ERRPx("Kernel Top is 0x%p, Physical Memory is 0x%p\n", kernel_top,
              processor_physmem);
        DEBUGP_EXIT;
        return (error);
    }

    DEBUGP_EXIT;
    return (0);
}

/*******************************************************************
 * ABORT any DMA in progress                                       *
 *******************************************************************/
void ai64_abort_any_dma_inprogress(struct ai_board *ai64_device)
{
    int dma_cmd_stat;
    int dma_mode;

    DEBUGP_ENTER;
    writelocal(ai64_device, 0, AI64_INT_CTRL_REG);
    // Clear the local Interrupt Control Reg.
    /* ABORT ANY DMA IN PROGRESS */
    dma_cmd_stat = readl(DMACmdStatus(ai64_device));
    dma_mode = readl(DMAMode0(ai64_device));

    if ((dma_cmd_stat & CH0_DMA_ENABLE_MASK) &&
        !(dma_cmd_stat & CH0_DMA_DONE_MASK)) {
        writel(DMA_ABORT_CMD_0, DMACmdStatus(ai64_device));
        ai64_device->dmainprogress = NO_DMA_INPROGRESS;
        DEBUGPx(D_ERR, "ISSUING DMA ABORT: dma_cmd_stat=0x%x\n",
                dma_cmd_stat);
        /* wakeup any blocked read threads */
        DEBUGP(D_WAIT, "wake_up dmawq\n");
        wake_up_interruptible(&ai64_device->dmawq);
    }
				
	/* DMA would not start after a timeout */
    writel(SETUP_DMA_CMD_0, DMACmdStatus(ai64_device));
    ai64_device->wait_for_dma_to_complete = 0;
    DEBUGP_EXIT;
}


/*******************************************************************
 * Fire DMA                                                        *
 *******************************************************************/
int ai64_fire_dma(struct ai_board *ai64_device)
{
    unsigned long   regval;

    DEBUGP_ENTER;

    if(ai64_device->num_dma_buffers == 0) {
        DEBUGP(D_ERR, "No DMA buffers specified\n");
        DEBUGP_EXIT;
        return (-ENOBUFS);
    }

    /* sanity check - should never happen */
    if (ai64_device->dmainprogress == DMA_INPROGRESS) {
        DEBUGP(D_ERR, "##### DMA ALREADY IN PROGRESS #####\n");
        DEBUGP_EXIT;
        return (0);
    }

    ai64_device->read_to_issue_dma = 0;
    ai64_device->dmainprogress = DMA_INPROGRESS;

    /*** This is a klooge to check for empty DMA read ***/
    ai64_device->dma[ai64_device->cbuffer_w].dmabufferpool[0] = 0xDEADBEEF;

    writel(0, DMAThreshold(ai64_device));   // #1 DMA Threshold 0xB0

    // writel(DEMAND_DMA_MODE, DMAMode0(ai64_device));   // #2 Mode 0x80
    writel(NON_DEMAND_DMA_MODE, DMAMode0(ai64_device)); // Mode 0x80

    // Set up DMA Address & Count
    writel(ai64_device->dma[ai64_device->cbuffer_w].dmaoffset,
           DMAPCIAddr0(ai64_device));
    // #3 PCI Address 0x84
    writel((AI64_INPUT_DATA_BUFF_REG << 2), DMALocalAddr0(ai64_device));
    // #4 Local Address 0x88
    writel((((ai64_device->dmaSampleSize) << 2)),
           DMAByteCnt0(ai64_device));
    // #5 Xfer Byte Count 0x8C
    // writel(0xA, DMADescrPtr0(ai64_device));     // #6 Descriptor End of Chain 0x90
    writel(0xC, DMADescrPtr0(ai64_device));
    // #6 Descriptor Int after TCount 0x90  ***GT***

    // writel(0, DMAArbitr(ai64_device)); // #7 MARB Mode/Arbitration (RRobin) 0xAC 
    writel((1 << 19), DMAArbitr(ai64_device));
    // #7 MARB Mode/Arbitration (DMA0 Pri) 0xAC   **GT**

    regval = readl(DMACmdStatus(ai64_device));  // DMA CSR 0xA8
    // regval &= STOP_DMA_CMD_0_MASK;       // DRD

    regval &= ~STARTUP_DMA_CMD_0;   // DRD
    writel(regval, DMACmdStatus(ai64_device));
    // #3 CLEAR STARTUP Bits 0,1 DMA CSR 0xA8

    // Clear interrupts
    writel((regval | DMA_CMD_STAT_INT_CLEAR), DMACmdStatus(ai64_device));
    // DMA CSR 0xA8

    DEBUGPx(D_L4, "fifosamples=%d dmaSampleSize=%d\n",
            readlocal(ai64_device, AI64_BUFFER_SIZE_REG),
            (ai64_device->dmaSampleSize));

    // Enable the DMA
	/* DMA would not start after a timeout */
    writel((regval | SETUP_DMA_CMD_0), DMACmdStatus(ai64_device));    // DMA CSR 0xA8

    /*** Check for FIFO overflow ***/
    ai64_device->dma[ai64_device->cbuffer_w].fifo_read =
        readlocal(ai64_device, AI64_BUFFER_SIZE_REG);

    /***
	 *** if we do not have enough samples in the FIFO, wait till the  
	 *** requested number of samples have been filled prior to issuing
	 *** the dma.
	 ***/
#if 1
     if(ai64_device->dmaSampleSize >
                    ai64_device->dma[ai64_device->cbuffer_w].fifo_read)  {
        writelocal(ai64_device, AI64_ICR_1_LO_HI, AI64_INT_CTRL_REG); /* enable interrupts */
        ai64_device->dmainprogress = NO_DMA_INPROGRESS;
        DEBUGPx(D_L3, "### WAIT FOR SAMPLES: fifo_read=0x%x "
                "dmaSampleSize=0x%x\n",
                ai64_device->dma[ai64_device->cbuffer_w].fifo_read,
                ai64_device->dmaSampleSize);
        DEBUGP_EXIT;
        return (0);
    }
#else
    if ((ai64_device->dmaSampleSize > 2 * ai64_device->nchans) &&
        (ai64_device->dma[ai64_device->cbuffer_w].fifo_read <
         (ai64_device->dmaSampleSize - 2 * ai64_device->nchans))) {
        writelocal(ai64_device, AI64_ICR_1_LO_HI, AI64_INT_CTRL_REG); /* enable interrupts */
        ai64_device->dmainprogress = NO_DMA_INPROGRESS;
        DEBUGPx(D_L3, "### WAIT FOR SAMPLES: fifo_read=0x%x "
                "dmaSampleSize=0x%x\n",
                ai64_device->dma[ai64_device->cbuffer_w].fifo_read,
                ai64_device->dmaSampleSize);
        DEBUGP_EXIT;
        return (0);
    }
#endif

    /*** check for overflow of fifo ***/
    if (ai64_device->dma[ai64_device->cbuffer_w].fifo_read >=
        (MAX_DMA_SAMPLES - ai64_device->nchans)) {
        DEBUGPx(D_ERR, "- AI64_DMA_CONTINUOUS MODE: FIFO overflow - "
                "read too slow (fifo=0x%x)\n",
                ai64_device->dma[ai64_device->cbuffer_w].fifo_read);
        ai64_device->error = AI64_DEVICE_FIFO_OVERFLOW;
        ai64_device->dma_abort_request++;
        ai64_device->dma[ai64_device->cbuffer_w].bufferstate = DmaOverflow;
        regval = readl(DMACmdStatus(ai64_device));  // DMA CSR 0xA8
        regval &= ~STARTUP_DMA_CMD_0;
        writel(regval, DMACmdStatus(ai64_device));
        // #3 CLEAR STARTUP Bits 0,1 DMA CSR 0xA8
    }

    /* if hardware has been initialized, skip starting DMA */
    if (ai64_device->hw_init_during_read || ai64_device->dma_abort_request) {
        ai64_device->dmainprogress = NO_DMA_INPROGRESS;
        DEBUGP(D_ERR,
                "HW Initialized or DMA abort request prior to starting DMA\n");
        DEBUGP_EXIT;
        if(ai64_device->dma_abort_request)
            return (-ENOBUFS);
        else
            return (-EINTR);
    }

    ai64_device->WriteCount++;
    DEBUGPx(D_L4, "WRITE - size=%d, WriteCount=%d\n",
            (ai64_device->dmaSampleSize << 2), ai64_device->WriteCount);

    DEBUGP(D_DMA, "########## DMA BEING STARTED ##########\n");
    // Start the DMA
    writel((regval | STARTUP_DMA_CMD_0), DMACmdStatus(ai64_device));    // DMA CSR 0xA8

    DEBUGPx(D_L3, "START_DMA *** dma icr1 lo-hi:  **********  "
            "CBUFFER_W %d **********\n", ai64_device->cbuffer_w);
    DEBUGP_EXIT;
    return (0);
}

/*******************************************************************
 * Compute new buffer in use HWM (For DEBUG Only)                  *
 *******************************************************************/
void ai64_hwm_check(struct ai_board *ai64_device)
{
    int delta;

    if (ai64_device->cbuffer_w < ai64_device->cbuffer_r)
        delta = ai64_device->num_dma_buffers + ai64_device->cbuffer_w
            - ai64_device->cbuffer_r;
    else
        delta = ai64_device->cbuffer_w - ai64_device->cbuffer_r;

    delta++;

    if (ai64_device->buf_inuse_hwm < delta) {
        ai64_device->buf_inuse_hwm = delta;
        DEBUGPx(D_HWM, "### DMA Buffers HWM ###: "
                "In_use=%2d (total=%2d cbuffer_r=%2d cbuffer_w=%2d)\n",
                ai64_device->buf_inuse_hwm, ai64_device->num_dma_buffers,
                ai64_device->cbuffer_r, ai64_device->cbuffer_w);
    }
}

/*******************************************************************
 * Allocate physical memory for DMA                                *
 *******************************************************************/
int ai64_allocate_physical_memory(struct ai_board *ai64_device, int size)
{
    unsigned long virt_mem;
    int i, npages;

    DEBUGP_ENTER;

    /* if memory already allocated, return */
    if (ai64_device->phys_mem) {
        DEBUGPx(D_L2,
                "Physical memory at 0x%p, size=%d already allocated\n",
                ai64_device->phys_mem, ai64_device->phys_mem_size);
        DEBUGP_EXIT;
        return (-EADDRINUSE); /* good return */
    }

    if (size == 0) {
        DEBUGP(D_ERR, "Physical Memory size of 0 is invalid\n");
        ai64_device->error = AI64_DEVICE_INVALID_PARAMETER;
        DEBUGP_EXIT;
        return -EINVAL; /* error return */
    }

    /*** Now allocate physical memory ***/
    /* round to page size */
    ai64_device->phys_mem_size = (size + PAGE_SIZE - 1) / PAGE_SIZE *
        PAGE_SIZE;
    ai64_device->phys_mem_size_freed = 0;
    ai64_device->virt_mem = (u32 *) __get_free_pages(GFP_KERNEL | GFP_DMA32,
                                                     get_order
                                                     (ai64_device->
                                                      phys_mem_size));

    DEBUGPx(D_L1, "virt_mem=%p size=0x%x\n",
            ai64_device->virt_mem, ai64_device->phys_mem_size);

    if (ai64_device->virt_mem == 0) {
        DEBUGPx(D_ERR, "can not allocate DMA memory of size %d bytes\n",
                ai64_device->phys_mem_size);
        ai64_device->error = AI64_DEVICE_RESOURCE_ALLOCATION_ERROR;
        ai64_device->phys_mem_size = 0;
        ai64_device->phys_mem_size_freed = 0;
        DEBUGP_EXIT;
        return (-ENOMEM);
    }

    /* reserve pages so that DMA will work */
    virt_mem = (unsigned long) ai64_device->virt_mem;
    npages = ai64_device->phys_mem_size / PAGE_SIZE;
    for (i = 0; i < npages; i++) {
        SetPageReserved(virt_to_page(virt_mem));
        virt_mem += PAGE_SIZE;
    }
    ai64_device->phys_mem =
        (u32 *)(unsigned long) virt_to_phys((void *) ai64_device->virt_mem);

    DEBUGPx(D_L1, "phys_mem=%p\n", ai64_device->phys_mem);
    if ((ai64_device->phys_mem == 0) || ((unsigned long)ai64_device->phys_mem > 0xFFFFFFFFL)) {
        ai64_free_physical_memory(ai64_device);
		if(ai64_device->phys_mem) {
           printk (KERN_INFO AI64_DEVICE_NAME 
               ": memory allocation failed: "
               "kernel returned memory above 4GB (0x%p)\n",   
                ai64_device->phys_mem);
            DEBUGPx(D_ERR, "virt_to_phys returned physical memory (%p) "
                     "beyond 4GB)\n", ai64_device->phys_mem);
        }
        else
            DEBUGP(D_ERR, "virt_to_phys returned zero physical memory\n");
        ai64_device->error = AI64_DEVICE_RESOURCE_ALLOCATION_ERROR;
        DEBUGP_EXIT;
        return (-ENOMEM);
    }

    DEBUGP_EXIT;
    return (0);
}

/*******************************************************************
 * Free physical memory                                            *
 *******************************************************************/
void ai64_free_physical_memory(struct ai_board *ai64_device)
{
    unsigned long virt_mem;
    int i, npages;

    DEBUGP_ENTER;
    if (ai64_device->virt_mem) {
        DEBUGPx(D_L1, "Freeing Physical Memory at virt=%p phys=%p size=%d\n",
            ai64_device->virt_mem, ai64_device->phys_mem,
            ai64_device->phys_mem_size);
        virt_mem = (unsigned long) ai64_device->virt_mem;
        npages = ai64_device->phys_mem_size / PAGE_SIZE;
        for (i = 0; i < npages; i++) {
            ClearPageReserved(virt_to_page(virt_mem));
            virt_mem += PAGE_SIZE;
        }

        /* free requested buffer */
        free_pages((unsigned long) ai64_device->virt_mem,
                   get_order(ai64_device->phys_mem_size));
        ai64_device->virt_mem = (u32 *) NULL;
        ai64_device->phys_mem = (u32 *) NULL;
        ai64_device->phys_mem_size = 0;
        ai64_device->phys_mem_size_freed = 0;
    }
        
    DEBUGP_EXIT;
}

/*******************************************************************
 * Validate Channel 0 position. 16/32 BIT operation                *
 *******************************************************************/
int ai64_validate_data(struct ai_board *ai64_device)
{
    int validate, index;
    ushort *usp;

    DEBUGP_ENTER;

    if  (ai64_device->validate_chan_0 &&            /* validation request*/
        (ai64_device->enable_test_pattern == 0) &&  /* not pattern gen */
        (ai64_device->nchans > 1))                  /* not single chan */
        validate = 1;           /* validate buffer */
    else
        validate = 0;           /* do not validate buffer */

    /*** if we are to validate channel 0 ***/
    if (validate) {
        /*** if 16bit samples requested ***/
        if (ai64_device->return_16bit_samples)
            usp = (ushort *) ai64_device->dmabuffer;
        else
            usp = 0;

        for (index = 0; index < ai64_device->dmaSampleSize; index++) {
            if (ai64_device->chan0_index) { /* not channel zero */
                if (ai64_device->dmabuffer[index] & 0x10000) {
                    DEBUGPx(D_ERR,
                            "Channel 0 data 0x%08x at wrong offset "
                            "%d: buf# %d (FIFO: at_dma=0x%x, now=0x%x)\n",
                            ai64_device->dmabuffer[index], index,
                            ai64_device->cbuffer_r,
                            ai64_device->dma[ai64_device->cbuffer_r].
                            fifo_read, readlocal(ai64_device,
                                                 AI64_BUFFER_SIZE_REG));
                    break;
                }
            } else {    /* channel zero */
                if (!(ai64_device->dmabuffer[index] & 0x10000)) {
                    DEBUGPx(D_ERR,
                            "Not Channel 0 data 0x%08x at offset %d: buf# %d"
                            " (FIFO: at_dma=0x%x, now=0x%x))\n",
                            ai64_device->dmabuffer[index], index,
                            ai64_device->cbuffer_r,
                            ai64_device->dma[ai64_device->cbuffer_r].
                            fifo_read, readlocal(ai64_device,
                                                 AI64_BUFFER_SIZE_REG));
                    break;
                }
            }
            ai64_device->chan0_index++; /* increment index */
            if (ai64_device->chan0_index == ai64_device->nchans)
                ai64_device->chan0_index = 0;   /* channel zero */

            /*** if 16bit samples requested ***/
            if (usp)
                usp[index] = ai64_device->dmabuffer[index];
        }

        if (index != ai64_device->dmaSampleSize) {  /* if error occurred */
            ai64_device->dma[ai64_device->cbuffer_r].bufferstate =
                DmaFree;
            ai64_abort_any_dma_inprogress(ai64_device);
            DEBUGP_EXIT;
            return (-EIO);
        }
    }
    //
    // Move the data - Must read all the data in the buffer!!
    //

    /* if user requests to receive 16 bit samples (strip the high
     * half word). This section of code is only entered if we
     * do not request channel validation. If we request channel
     * validation and return_16bit_samples is set, the copy is 
     * performed in the above routine. This way, we do not go
     * through the buffer twice.
     */
    if (ai64_device->return_16bit_samples && !validate) {
        usp = (ushort *) ai64_device->dmabuffer;
        for (index = 0; index < ai64_device->dmaSampleSize; index++)
            usp[index] = ai64_device->dmabuffer[index];
    }

    return (0);
}

module_init(ai64_init);
module_exit(ai64_exit);
