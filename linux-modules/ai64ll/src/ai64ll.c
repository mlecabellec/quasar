// vim:ts=4:
/******************************************************************************
 *                                                                            *
 * File:         ai64ll.c                                                     *
 *                                                                            *
 * Description:  Linux Host driver for AI64LL card. This driver is            *
 *               an installable module.It supports the 16AI64LL (16-bit)      *
 *               Analog Input card.                                           *
 *                                                                            *
 * Revision:     7.2_0                                                        *
 * Date:         02/15/2018                                                   *
 * History:                                                                   *
 *                                                                            *
 *  15  02/15/2018 P. Ongini                                                  * 
 *      provide support for RedHawk 7.2                                       *
 *                                                                            *
 *  14  12/15/2014 D. Dubash                                                  * 
 *      The driver only uses interrupts for board Initialization complete     *
 *      and AutoCal complete. The driver has been made more robust so as      *
 *      as to poll in case the interrupt fails to operate properly on         *
 *      some motherboards.                                                    *
 *                                                                            *
 *  13  11/29/2012 D. Dubash                                                  * 
 *      provide support for 6.3.1                                             *
 *                                                                            *
 *  12  11/29/2012 D. Dubash                                                  * 
 *      remove the unnecessary remove_proc_entry() in ai64ll_init().          *
 *                                                                            *
 *  11  07/21/2009 D. Dubash                                                  * 
 *      Fixed AI64LL/AI64 detection method as it was failing detection of     *
 *      an AI64 card because the card had a non-zero conversion value in it.  *
 *                                                                            *
 *  10  1/13/09 D. Dubash                                                     *
 *      Support for RH 5.2.x                                                  *
 *                                                                            *
 *   9  9/02/08 D. Dubash                                                     *
 *      Support for RH 5.1.x                                                  *
 *                                                                            *
 *   8  3/07/07 D. Dubash                                                     *
 *      Support for 64 bit architecture                                       *
 *                                                                            *
 *   7  3/01/07 D. Dubash                                                     *
 *      Use converter counter to distinguish between AI64 and AI64LL cards.   *
 *                                                                            *
 *   6  2/22/07 D. Dubash                                                     *
 *      Add support for RH4.1.7                                               *
 *                                                                            *
 *   5  2/09/07 D. Dubash                                                     *
 *      Add support for rpm.                                                  *
 *                                                                            *
 *   4  1/11/07 D. Dubash                                                     *
 *      Integrated with RedHawk 4.1.4. Updated driver to version 4.1.4_0      *
 *      Replaced IOCTL_MMAP_SELECT ioctl with IOCTL_AI64LL_MMAP_GET_OFFSET.   *
 *      Also, added code to skip AI64SS cards. Added more information to      *
 *      /proc/ai64ll.                                                         *
 *                                                                            *
 *   3  4/08/05 D. Dubash                                                     *
 *      Integrated with RedHawk 2.2. Updated driver to version 2.20           *
 *                                                                            *
 *   2  7/10/03 D. Dubash                                                     *
 *      Integrated with RedHawk 1.3. Updated driver to version 1.30           *
 *                                                                            *
 *   1  5/08/03 D. Dubash                                                     *
 *      Initial release                                                       *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *  Copyright (C) 2003 and beyond Concurrent Real-Time, Inc.                  *
 *  All rights reserved                                                       *
 *                                                                            *
 ******************************************************************************/

/*
 * File : ai64ll.c
 *
 * Comments :
 *
 * Revision History :
 * 08-21-02: Initial release
 *
 */

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
//#include <linux/config.h>
//#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/slab.h>

#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <asm/irq.h>
#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>

#include <asm/io.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,5,4)
#include <asm/system.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#include <linux/uaccess.h>
#endif
#include <asm/uaccess.h>
#include <asm/signal.h>
#include <linux/errno.h>

#include <linux/seq_file.h> /* procfs support */

#include "ai64ll.h"
#include "ai64ll_driver.h"

MODULE_LICENSE("Proprietary");

#define AI64LL		"ai64ll"
#define MODULE_NAME	AI64LL
#define PATTERN                     0x1234

static irqreturn_t ai64ll_irq_handler (int, void *);

static int fasync_ai64ll (int, struct file *, int);

int  ai64ll_configure_mem_regions(struct pci_dev *pdev, int which, 
					u32 *address, u32 *size, u32 *flags);
void ai64ll_deconfigure_mem_regions(int which, u32 address, u32 size, 
							u32 flags);
static void ai64ll_exit ( void );

/**********************************************************
debug ROUTINES
***********************************************************/
#ifdef	AI64LL_DEBUG

#define	DbgHdr	"%s(%4d): " 

#define DEBUGP0(Str)														\
    do {																	\
  		printk (KERN_INFO DbgHdr Str, __FILE__, __LINE__);				 	\
    }while(0)

#define DEBUGP1(Str,a1)														\
    do {																	\
  		printk (KERN_INFO DbgHdr Str, __FILE__, __LINE__, a1);			 	\
    }while(0)

#define DEBUGP2(Str,a1,a2)													\
    do {																	\
  		printk (KERN_INFO DbgHdr Str, __FILE__, __LINE__, a1, a2);		 	\
    }while(0)

#define DEBUGP3(Str,a1,a2,a3)												\
    do {																	\
  		printk (KERN_INFO DbgHdr Str, __FILE__, __LINE__, a1, a2, a3);		\
    }while(0)

#define DEBUGP4(Str,a1,a2,a3,a4)											\
    do {																	\
  		printk (KERN_INFO DbgHdr Str, __FILE__, __LINE__, a1, a2, a3, a4);	\
    }while(0)

#define DEBUGP5(Str,a1,a2,a3,a4,a5)											\
    do {																	\
  		printk (KERN_INFO DbgHdr Str, __FILE__, __LINE__, a1, a2, a3, 		\
				a4, a5);													\
    }while(0)
#else	/* else AI64LL_DEBUG */
#define DEBUGP0(Str)
#define DEBUGP1(Str,a1)
#define DEBUGP2(Str,a1,a2)
#define DEBUGP3(Str,a1,a2,a3)
#define DEBUGP4(Str,a1,a2,a3,a4)
#define DEBUGP5(Str,a1,a2,a3,a4,a5)
#endif	/* end AI64LL_DEBUG */

/**********************************************************
error ROUTINES
***********************************************************/
#define	ErrHdr	"%s(%4d): ERROR!!!: " 

#define ERRP0(Str)															\
    do {																	\
  		printk (KERN_ERR ErrHdr Str, __FILE__, __LINE__);				 	\
    }while(0)

#define ERRP1(Str,a1)														\
    do {																	\
  		printk (KERN_ERR ErrHdr Str, __FILE__, __LINE__, a1);			 	\
    }while(0)

#define ERRP2(Str,a1,a2)													\
    do {																	\
  		printk (KERN_ERR ErrHdr Str, __FILE__, __LINE__, a1, a2);		 	\
    }while(0)

#define ERRP3(Str,a1,a2,a3)													\
    do {																	\
  		printk (KERN_ERR ErrHdr Str, __FILE__, __LINE__, a1, a2, a3);		\
    }while(0)

#define ERRP4(Str,a1,a2,a3,a4)												\
    do {																	\
  		printk (KERN_ERR ErrHdr Str, __FILE__, __LINE__, a1, a2, a3, a4);	\
    }while(0)

#define ERRP5(Str,a1,a2,a3,a4,a5)											\
    do {																	\
  		printk (KERN_ERR ErrHdr Str, __FILE__, __LINE__, a1, a2, a3, 		\
				a4, a5);													\
    }while(0)


static struct file_operations ai64ll_fops = {
	read:		ai64ll_read,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
    unlocked_ioctl:unlocked_ai64ll_ioctl,
#else
	ioctl:  	ai64ll_ioctl,
#endif
	open:		ai64ll_open,
	release:	ai64ll_release,
	fasync:		fasync_ai64ll,
#ifdef AI64LL_MMAP_SUPPORT	/* drd */
    mmap:       ai64ll_mmap,
#endif /* end AI64LL_MMAP_SUPPORT */
};

static int ai64ll_proc_show(struct seq_file *m, void *v);

static int ai64ll_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, ai64ll_proc_show, NULL);
}

#if   	 LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
static struct proc_ops ai64ll_proc_operations = {
        .proc_open           = ai64ll_proc_open,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static struct file_operations ai64ll_proc_operations = {
        .owner          = THIS_MODULE,
        .open           = ai64ll_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

static int proc_enabled=0;
static	int	board_count	= 0;	/* Total number of boards found. */
static struct ai64ll_device *boards_dev = 0;
static struct ai64ll_device *ai64ll_device_list[MAX_BOARDS];
static int major = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
long unlocked_ai64ll_ioctl(struct file *filp,u_int iocmd,unsigned long ioarg )
{
    int ret;        
    struct inode *inode;
    struct ai64ll_device *device = (struct ai64ll_device *)filp->private_data;
                    
    DEBUGP1("unlocked_ai64ll_ioctl(): (%d): unlocked_ai64ll_ioctl() entered...\n", device->minor);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
    inode = filp->f_path.dentry->d_inode;
#else
    inode = filp->f_dentry->d_inode;
#endif
                    
    mutex_lock(&device->ioctl_mtx);
    if(device->ioctl_processing) {
        DEBUGP0("unlocked_ai64ll_ioctl(): ioctl processing active: busy\n");
        mutex_unlock(&device->ioctl_mtx);
        return (-EBUSY);    /* DO NOT USE IOCTL_RETURN() CALL HERE */
    }               
    mutex_unlock(&device->ioctl_mtx);

    ret = ai64ll_ioctl(inode, filp, iocmd, ioarg);
                    
    return( (long) ret );
}                   
#endif   

#define IOCTL_RETURN(Code) {     \
    ai64ll_device->ioctl_processing = 0; \
    return (Code);  \
}

/* procfs support */
static int ai64ll_proc_show(struct seq_file *m, void *v)
{
    int  bcnt;
	struct ai64ll_device *ai64ll_device;

	seq_printf(	m,
				"version: %s\n"
				"built: %s, %s\n"
				"boards: %d\n",
				AI64LL_VERSION,
				__DATE__, __TIME__,
				board_count);

    for (bcnt = 0; bcnt < board_count; bcnt++) {
        ai64ll_device = ai64ll_device_list[bcnt];
        seq_printf(m, "  card=%d: [%02x:%02x.%1d] bus=%d, slot=%d, func=%d, "
                    "irq=%d, Firmware=0x%x\n",
                    bcnt, ai64ll_device->bus, ai64ll_device->slot,
                    ai64ll_device->func, ai64ll_device->bus,ai64ll_device->slot,
                    ai64ll_device->func, ai64ll_device->irq,
                    ai64ll_device->firmware);
    }

    seq_puts(m,"\n");

    return 0;
}


#if 0
/*************************************************************************
* _proc_read - provide /proc file system information
*
* This function is called to provide the /proc file system data for this
* driver.
*
* RETURNS : number of chars put in buffer
*/
static int _proc_read(
	char*	page,
	char**	start,
	off_t	off,
	int	    count,
	int*	eof,
	void*	data)
{
    char *cp;
    int  bcnt;
	struct ai64ll_device *ai64ll_device;

	#define	_PROC_MSG_SIZE	128

	#if PAGE_SIZE < _PROC_MSG_SIZE
	#error	/proc file size may be too small.
	#endif
	int	i;

	i	= sprintf(	page,
				"version: %s\n"
				"built: %s, %s\n"
				"boards: %d\n",
				AI64LL_VERSION,
				__DATE__, __TIME__,
				board_count);

    cp = &page[i];
    for (bcnt = 0; bcnt < board_count; bcnt++) {
        ai64ll_device = ai64ll_device_list[bcnt];
        i = sprintf(cp, "  card=%d: [%02x:%02x.%1d] bus=%d, slot=%d, func=%d, "
                    "irq=%d, Firmware=0x%x\n",
                    bcnt, ai64ll_device->bus, ai64ll_device->slot,
                    ai64ll_device->func, ai64ll_device->bus,ai64ll_device->slot,
                    ai64ll_device->func, ai64ll_device->irq,
                    ai64ll_device->firmware);
        cp += i;
    }

    i = sprintf(cp,"\n");
    cp += i;
    *cp = 0;

    i = strlen(page) + 1;

    if (i >= PAGE_SIZE) {
        DEBUGP0("ai64ll: _proc_read(): /proc/xxx is larger than PAGE_SIZE\n");
        i = PAGE_SIZE - 1;
    }

    i--;

	eof[0]	= 1;
	return(i);
}

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
int proc_get_info(
				  char*	page,
				  char**	start,
				  off_t	offset,
				  int	count,
				  int dummy)
{
	int	eof;
	int	i;
	
	i	= _proc_read(page, start, offset, count, &eof, NULL);
	return(i);
}
#endif

/*
 * File : ai64ll_init ()
 *
 * Description : This function is for installing the driver module.
 * This function probes for the AI64LL device and then registers
 * the driver to the kernel.
 *
 * Input parameters : None
 *
 * Return Value : Returns zero, when success.
 * 		  otherwise, it returns an error number belonging to
 *                the error codes defined in <linux/errno.h>
 *
 * Comments : This function gets invoked, whenever the driver module
 * is installed using the linux command <insmod>
 *
 * Revision History :  pci_for_each_dev() function is used for 2.4 kernel.
 *                     pdev->resource[] fields are used to get the virt address.
 *                     wait_queue initialization modified for 2.4 Kernel.
 */

static int
ai64ll_init (void)
{
	struct pci_dev *pdev = NULL;
	struct ai64ll_device *ai64ll_device_next;
    u32             regval;
    int             region_error;
	int             i;
	int 			not_ai64ll_board;
  
	/* Allocate the resources for the ai64ll_device structure */
  
	struct ai64ll_device *ai64ll_device = 0;


	/* Probe through all the pci devices found in the pci slots */
//	while ((pdev = pci_find_device (AI64LL_VENDOR_ID, AI64LL_DEVICE_ID, pdev))) {
	while ((pdev = pci_get_device (AI64LL_VENDOR_ID, AI64LL_DEVICE_ID, pdev))) {

			if (pci_enable_device (pdev))
				continue;

			/* Probe for AI64LL card in all the pci slots available using Device ID
			and Vendor ID */

			ai64ll_device =
				(struct ai64ll_device *) kmalloc (sizeof(struct ai64ll_device),
																GFP_KERNEL);

            if(ai64ll_device == 0) {
                printk("AI64LL: Error: cannot allocate ai64ll_device structure\n");
                continue;
            }

            memset(ai64ll_device, 0, sizeof(struct ai64ll_device));
                                                        /* zero structure */

			DEBUGP1("ai64ll_init(): ai64ll_device = 0x%x\n",(int)ai64ll_device);

			/*
			 * A General Standard card is found, Probe if the board is 12AI64LL
			 * or 16AI64LL by reading the Subsystem id
			 */

			/*** fixed to get the correct subsys_id - drd ***/
			pci_read_config_dword(pdev, (unsigned long) PCI_SUBSYSTEM_VENDOR_ID,
									&ai64ll_device->subsys_id);

			/*Set the board id field in the ai64ll_device structure if present*/
			if (ai64ll_device->subsys_id != AI64LL_SUBSYS_ID) {
				/* card not found... continue */
				ai64ll_device->subsys_id = 0;
				kfree (ai64ll_device);
				continue;
			}

			DEBUGP3 ("Device id = %x; Vendor id = %x; Subsystem id = %x\n", 
					ai64ll_device->device_id, ai64ll_device->vendor_id, 
					ai64ll_device->subsys_id);
			DEBUGP0 ("AI64LL card found\n");
       
			/* set vendor and device id */
			ai64ll_device->vendor_id = pdev->vendor;
			ai64ll_device->device_id = pdev->device;

            region_error = 0;   /* set success flag */
			/*** mark all the memory regions in use ***/
			for(i = 0; i < AI64LL_MAX_REGION; i++) {
				if(ai64ll_configure_mem_regions(pdev, i, 
					&ai64ll_device->mem_region[i].address,
					&ai64ll_device->mem_region[i].size,
					&ai64ll_device->mem_region[i].flags)) {

					/*** If configuration failed, we need to remove the
					 *** memory requested
					 ***/
					/* remove requested memory regions */
					for(i = 0; i < AI64LL_MAX_REGION; i++) {
					    ai64ll_deconfigure_mem_regions(i, 
						    ai64ll_device->mem_region[i].address,
						    ai64ll_device->mem_region[i].size,
						    ai64ll_device->mem_region[i].flags);
					}
					kfree (ai64ll_device);
                    region_error++;   /* mark error */
				}
			}

            if(region_error)    /* if region failed, skip card */
                continue;

			/*** remap physical to virtual ***/
			/*** PCI_BASE_ADDRESS_0 ***/
			ai64ll_device->config_reg_address =
#if 	 LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
					(u32 *) ioremap (
						   ai64ll_device->mem_region[CONFIG_REGION].address,
						   ai64ll_device->mem_region[CONFIG_REGION].size);
#else
					(u32 *) ioremap_nocache (
						   ai64ll_device->mem_region[CONFIG_REGION].address,
						   ai64ll_device->mem_region[CONFIG_REGION].size);
#endif

			/*** PCI_BASE_ADDRESS_2 ***/
			ai64ll_device->local_reg_address = 
#if 	 LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
						(u32 *) ioremap (
							ai64ll_device->mem_region[LOCAL_REGION].address,
							ai64ll_device->mem_region[LOCAL_REGION].size);
#else
						(u32 *) ioremap_nocache (
							ai64ll_device->mem_region[LOCAL_REGION].address,
							ai64ll_device->mem_region[LOCAL_REGION].size);
#endif

            regval = readl(FirmwareRevision);
            /* if firmware value is bad, error out */
            if (regval == 0xffffffff) {

                /* Unmap the memory space allocated for the device found */
                iounmap(ai64ll_device->local_reg_address);
                iounmap(ai64ll_device->config_reg_address);

                /* remove requested memory regions */
				for(i = 0; i < AI64LL_MAX_REGION; i++) {
				    ai64ll_deconfigure_mem_regions(i, 
					    ai64ll_device->mem_region[i].address,
					    ai64ll_device->mem_region[i].size,
					    ai64ll_device->mem_region[i].flags);
				}

				kfree (ai64ll_device);
                ai64ll_device=NULL;

                printk("AI64LL: ERROR: PCI Bus# %d, Device# %d.%d: Failed!!!\n",
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

			/* disable the incrementing of the conversion counter */
			writel(SC_AIN_RESET_INPUTS_MASK, ScanSyncCtrlReg);
			writel(0, ConvCounter);	/* reset the counter */
			/* enable the incrementing of the conversion counter */
			writel(SC_AIN_ENABLE_SCAN_MASK, ScanSyncCtrlReg);
			not_ai64ll_board = 0;	/* assume ai64ll card */
			{
				#define TEST_RETRY 3
				int retry;
				int new_count, old_count;
				old_count=readl(ConvCounter);
				for(retry=0; retry < TEST_RETRY; retry++) {
					msleep_interruptible(1);
					if((new_count=readl(ConvCounter))<= old_count) {
						not_ai64ll_board++;
						if(not_ai64ll_board == 1) {
							printk (KERN_INFO AI64LL 
								": Device id = %x; Vendor id = %x; "
								"Subsystem id = %x\n", 
								ai64ll_device->device_id, 
								ai64ll_device->vendor_id, 
								ai64ll_device->subsys_id);

            				printk(KERN_INFO AI64LL 
								": /dev/%s%d: PCI Bus# %d, "
                   				"Device# %d.%d, Firmware: 0x%x\n",
                   				AI64LL, ai64ll_device->minor, pdev->bus->number,
                   				PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn),
                   				(u32)regval);
						}
						printk(KERN_INFO AI64LL
								": retry %d: ### NOT AI64LL BOARD ### : "
								"conversion counter = %d\n",
								not_ai64ll_board, readl(ConvCounter));
					}
					old_count = new_count;
				}
			}

			/* disable the incrementing of the conversion counter */
			writel(SC_AIN_RESET_INPUTS_MASK, ScanSyncCtrlReg);
			if (not_ai64ll_board) {
                /* Unmap the memory space allocated for the device found */
                iounmap(ai64ll_device->local_reg_address);
                iounmap(ai64ll_device->config_reg_address);

                /* remove requested memory regions */
				for(i = 0; i < AI64LL_MAX_REGION; i++) {
				    ai64ll_deconfigure_mem_regions(i, 
					    ai64ll_device->mem_region[i].address,
					    ai64ll_device->mem_region[i].size,
					    ai64ll_device->mem_region[i].flags);
				}

				kfree (ai64ll_device);
                ai64ll_device=NULL;
                continue;
            }

			/* Initialize the ai64ll_device structure with default values */
#ifdef LINUX_2_4
			init_waitqueue_head (&ai64ll_device->ioctlwq);
#else /* else !LINUX_2_4 */
			ai64ll_device->ioctlwq = NULL;
#endif /* end LINUX_2_4 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
            mutex_init(&ai64ll_device->ioctl_mtx);
#endif
	  
			AI64LL_LOCK_INIT();		/* Initialize the SMP lock. drd */

			ai64ll_device->pdev = pdev;
			pci_dev_get(ai64ll_device->pdev);	/* increment reference count */

			ai64ll_device->irq = pdev->irq;
			ai64ll_device->busy = 0;
			ai64ll_device->minor = board_count;
			ai64ll_device->next = boards_dev;
			boards_dev = ai64ll_device;

            ai64ll_device_list[board_count] = ai64ll_device;
            ai64ll_device->bus = pdev->bus->number;   /* pci number */
            ai64ll_device->slot = PCI_SLOT(pdev->devfn);  /* slot number */
            ai64ll_device->func = PCI_FUNC(pdev->devfn);  /* function number */
            ai64ll_device->firmware = (u32)regval; /* firmware revision */
	  
	        /* clear debug buffer - drd */
	        for(i=0; i < AI64LL_DEBUG_SIZE; i++)
		        ai64ll_device->debug_buffer[i] = 0;
   
			/* Increment the number of boards found */
	  
			board_count++;

            printk(KERN_INFO AI64LL ": /dev/%s%d: PCI Bus# %d, "
                   "Device# %d.%d, Irq=%d, Firmware: 0x%x\n",
                   AI64LL, ai64ll_device->minor, ai64ll_device->bus,
                   ai64ll_device->slot, ai64ll_device->func, ai64ll_device->irq,
                   ai64ll_device->firmware);
	}

	/* If no boards are found return error */
	if (board_count == 0)
		return -ENODEV;		/* No Device is found */

	/*
	 * Register the driver to the kernel as a character
	 * driver using dynamic allocation of major number
	 */

	major = register_chrdev (0, MODULE_NAME, &ai64ll_fops);
  
	/* if registration of the driver has failed */
	if (major < 0) {
		for (ai64ll_device = boards_dev; ai64ll_device;
								ai64ll_device = ai64ll_device_next) {
			ai64ll_device_next = ai64ll_device->next;

			/* Unmap the memory space allocated for the device found */
			iounmap (&ai64ll_device->local_reg_address);
			iounmap (&ai64ll_device->config_reg_address);

			/* remove requested memory regions */
			for(i = 0; i < AI64LL_MAX_REGION; i++) {
				ai64ll_deconfigure_mem_regions(i, 
					ai64ll_device->mem_region[i].address,
					ai64ll_device->mem_region[i].size,
					ai64ll_device->mem_region[i].flags);
			}

            pci_dev_put(ai64ll_device->pdev); /* decrement reference counter */
			kfree (ai64ll_device);
		}
		boards_dev = NULL;
		return -ENODEV;		/* No device found */
	}


    if(board_count) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
        remove_proc_entry(MODULE_NAME, NULL); /* just in case */
#endif
        proc_enabled = 0;
#if 0 /* old way */
        {
	        struct proc_dir_entry*	entry;
		    entry	= create_proc_entry(MODULE_NAME, S_IRUGO, NULL);
		    if (entry) {
                proc_enabled = 1;
			    //entry->read_proc	= _proc_read;
                entry->proc_fops = &ai64ll_proc_operations;
		    }
		    else {
			    ERRP0("ai64ll_init: Could not create /proc entry\n");
			    return 0;
		    }
        }
#else
        if(!proc_create(MODULE_NAME, S_IRUGO, NULL, &ai64ll_proc_operations)) {
			ERRP0("ai64ll_init: Could not create /proc entry\n");
            ai64ll_exit();
            return (-ENOMEM);
        }
        proc_enabled = 1;
#endif
	}

	printk ("AI64LL driver version %s successfully inserted.\n",AI64LL_VERSION);
	return 0;
}

/*
 * File : ai64ll_exit ()
 *
 * Description : This function is for Un-installing the driver
 * module.This function unregisters the driver from the kernel.
 *
 * Input parameters : None
 *
 * Return Value : void.
 *
 * Comments : This function gets invoked, whenever the driver module is
 * un-installed using the linux command <rmmod>
 *
 */
static void
ai64ll_exit ( void )
{
	struct ai64ll_device *ai64ll_device_next;
	struct ai64ll_device *ai64ll_device ;
	int i;
  
	/* Unregister the driver to the kernel as a character driver */
  
	unregister_chrdev (major, MODULE_NAME);
	for (ai64ll_device = boards_dev;
		ai64ll_device; ai64ll_device = ai64ll_device_next) {
      
		DEBUGP1 ("ai64ll_exit(): ai64ll_device=0x%x\n",(u32)ai64ll_device);

		ai64ll_device_next = ai64ll_device->next;

		/* Unmap the memory space allocated for the device found */
		iounmap (&ai64ll_device->local_reg_address);
		iounmap (&ai64ll_device->config_reg_address);

		/* remove requested memory regions */
		for(i = 0; i < AI64LL_MAX_REGION; i++) {
			ai64ll_deconfigure_mem_regions(i, 
				ai64ll_device->mem_region[i].address,
				ai64ll_device->mem_region[i].size,
				ai64ll_device->mem_region[i].flags);
		}
        pci_dev_put(ai64ll_device->pdev); /* decrement reference counter */
		kfree (ai64ll_device);
        ai64ll_device = NULL;
	}
    if(proc_enabled) {
	    remove_proc_entry(MODULE_NAME, NULL);
        proc_enabled = 0;
    }
	printk ("AI64LL driver version %s successfully removed.\n",AI64LL_VERSION);
}

/*
 * File : ai64ll_open ()
 *
 * Description : This function is to enable access to the device.
 * It initializes the device, if it is being opened for the first time,
 * allocates all the resources needed by the driver and increments the
 * usage count.
 *
 * Input parameters : inode - Device node (structure) holding the device
 *                            information.
 *                    fops  - File pointer to the file operations
 *                            structure
 *
 * Return Value : Returns zero, when success.
 *                otherwise, it returns an error number belonging to the
 *                error codes defined in <linux/errno.h>
 *
 * Comments : This function is registered into the file_operation for
 * opening the device. Whenever open function is called for this device
 * node (file) in a linux application, this function gets invoked.
 *
 * Revision History : wait_queue initialization modified for 2.4 Kernel.
 * 22-12-01: Intialization of interrupt request status added.
 *
 */

int
ai64ll_open (struct inode *inode, struct file *fp)
{
	struct ai64ll_device *ai64ll_device=0;
	int		error;

	DEBUGP0 ("<open> function entry ...\n");

	/* Enable the access for all the ai64ll devices found */
	for (ai64ll_device = boards_dev; ai64ll_device;
		ai64ll_device = ai64ll_device->next) {
      
		/* check if the device is already opened using the minor number */

		if (MINOR (inode->i_rdev) == ai64ll_device->minor) {
			if ( ai64ll_device->busy ) {
				/* Device is already opened */
				ERRP0 ("Device is already opened\n");
				return -EBUSY;	/* Device is Busy */
			}

			/*
			 * Register the interrupt service routine for the irq
			 * line allocated for the device
			 */
      
			/*
			 * As this device can be placed in any slot that shares an IRQ with
			 * other devices, we need to add the SA_SHIRQ (share IRQ) flag
			 * as this will enable our device to share the same IRQ with other
			 * drivers. Without this flag, the request_irq() call would fail 
			 * with a BUSY (-16) status. We have no control of how the IRQs are
			 * set by the hardware. All we can do is to move the board into
			 * another slot to get (hopefully) a different IRQ.
			 */
			if ((error = request_irq
				(ai64ll_device->irq, &ai64ll_irq_handler, 
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0)
  				 				IRQF_DISABLED | IRQF_SHARED,
								MODULE_NAME, ai64ll_device)) < 0) {
#else
  				 				IRQF_SHARED,
								MODULE_NAME, ai64ll_device)) < 0) {
#endif

				/* Interrupt Service routine registration failed */
				/* Set the device status as busy */
				ERRP1 ("Register interrupt service routine failed. Error=%d\n",
						error);
				/* ai64ll_device->busy = 1; */
				return -EBUSY;	/* Device is Busy */
			}

			/* Initialize the ai64ll_device structure with default values */

#ifdef LINUX_2_4
			init_waitqueue_head (&ai64ll_device->ioctlwq);
#else /* else !LINUX_2_4 */
			ai64ll_device->ioctlwq = NULL;
#endif /* end LINUX_2_4 */
			ai64ll_device->busy = 1;
			ai64ll_device->minor = MINOR (inode->i_rdev);
			ai64ll_device->error = AI64LL_SUCCESS;
      
			ai64ll_device->int_notify.init	 = 0;
			ai64ll_device->int_notify.irq0	 = 0;
			ai64ll_device->int_status.init	 = 0;
			ai64ll_device->int_status.irq0	 = 0;

#ifdef AI64LL_MMAP_SUPPORT	/* drd - MMAP SUPPORT */
			ai64ll_device->mmap_reg_select = AI64LL_SELECT_GSC_MMAP;
#endif /* end AI64LL_MMAP_SUPPORT */

			fp->private_data = ai64ll_device;
			/* 
			 * Reset the DMA command status register
			 */
			writel ((DISABLE_DMA_CMD_0 | DISABLE_DMA_CMD_1), DmaCmdStatus);
      
			/* Mask the IRQ0 request flag in the IntCtrlReg */
			writel ((readl(IntCtrlReg)&IC_DISABLE_ALL_INTS_VAL),IntCtrlReg);

			/*
			 * Enable the local interrupts by writing PCI_INT_ENABLE
			 * Mask value to IntCtrlReg
			 */
			writel ((readl (IntCtrlStatus) | PCI_INT_ENABLE),IntCtrlStatus);
      
			/* Increment the module usage count */
			if (!try_module_get (THIS_MODULE)) {
				printk ("ai64ll_open: Unable to increment the mod count\n");
				return -EBUSY;
			}
			return 0;			/* Device open is Success */
		}
	}
  return -ENODEV;
}


/*
 * File : ai64ll_release ()
 *
 * Description : This function is to disable access to the device.
 * It frees all the resources allocted to the device in open() module
 * and decrements the usage count.
 *
 * Input parameters : inode - Device node (structure) holding the device
 *                            information.
 *                    fops  - File pointer to the file operations
 *                            structure.
 *
 * Return Value : Returns zero, when success.
 *                otherwise, it returns an error number belonging to the
 *                error codes defined in <linux/errno.h>
 *
 * Comments : This function is registered into the file_operation for
 * closing the device.Whenever close function is called for this device
 * node (file) in a linux application, this function gets invoked.
 *
 */

int
ai64ll_release (struct inode *inode, struct file *fp)
{
	struct ai64ll_device *ai64ll_device =
		(struct ai64ll_device *)fp->private_data;

	DEBUGP0 ("<release> function entry ...\n");
  
	/* Release the fasync pointer */
	fasync_ai64ll (-1, fp, 0);
  
	/* Clear the device busy state */
	ai64ll_device->busy = 0;
  
	/*
	 * Disable the local interrupts by writing ~PCI_INT_ENABLE
	 * Mask value to IntCtrlStatus - drd (original code was using the
	 * wrong register to disable.
	 */
	writel ((readl (IntCtrlStatus) & ~PCI_INT_ENABLE), IntCtrlStatus);
  
	/*
	 * Reset the DMA command status register
	 */
	writel ((DISABLE_DMA_CMD_0 | DISABLE_DMA_CMD_1), DmaCmdStatus);

	/* Reset the local irq flag in the IntCtrlReg */
	writel ((readl (IntCtrlReg) & IC_DISABLE_ALL_INTS_VAL), IntCtrlReg);
  
	/* Free the irq allocated for the device */
	free_irq (ai64ll_device->irq, ai64ll_device);
  
	/* Decrement the module usage count */
	module_put (THIS_MODULE);
	return 0;			/* Device Close is Success */
}

/*
 * File : ai64ll_read ()
 *
 * Description : This function is for reading the requested no. of bytes
 * of data from the input buffers (FIFO). This function can support DMA
 * operation and also buffered I/O operation as per the option selected
 * by the application through an ioctl call to the driver.
 *
 * Input parameters : fops  - File pointer to the file operations structure
 *                    buf   - Buffer to hold the data read from registers
 *                    size  - No. of bytes to be read ( No. of chans * 4 )
 *                    ppos  - Position, ( not used ).
 *
 * Return Value : Returns the number of bytes that have been read
 *                OR returns -EINVAL if no. of bytes to be read is invalid.
 *
 * Comments : This function is registered into the file_operation
 * to read data from the device. Whenever a read function for this device
 * is called in a linux application, this function gets invoked.
 *
 */

ssize_t
ai64ll_read (struct file *fp, char *buf, size_t size, loff_t *ppos)
{
	struct ai64ll_device*	ai64ll_device;
	unsigned int		chans_requested;
	int                   index;
  
	/* Get the device structure from the private_data field */
	ai64ll_device	= (struct ai64ll_device*) fp->private_data;

	DEBUGP0 ("<read> function entry ...\n");

	/* Get the requested no. of channels requested */
	chans_requested	= size / sizeof (int);
  
	/* Check if the number of channels requested to be read is valid */
	if ((chans_requested <= 0) || (size % sizeof (int)) || 
				(chans_requested > AI64LL_MAX_CHANS)) {
		/* Invalid no. of samples to be read */
		return 0;
	}

  
	/* Check if the buffer has write access */
	if (
#ifdef VERIFY_WRITE
        !access_ok (VERIFY_WRITE, (void*)buf,
					chans_requested*sizeof(int)) 
#else
        !access_ok ( (void*)buf,
					chans_requested*sizeof(int)) 
#endif
            ) {
        ai64ll_device->error = AI64LL_FAULT;
		return -EFAULT;
	}
    
	for(index = 0; index < AI64LL_MAX_CHANS; index++) {
		ai64ll_device->chan_regs[index] = 
				readl(ai64ll_device->local_reg_address +
					AI64LL_REGISTER_INDEX(AI64LL_GSC_CH(index)));
	}
    
	if(copy_to_user((void *)buf ,&ai64ll_device->chan_regs[0], size)) {
        ai64ll_device->error = AI64LL_FAULT;
		return -EFAULT;
    }
    
	return ( size );
}

/*
 * File : ai64ll_ioctl()
 *
 * Description : To send the device specific commands to the device.
 * The ioctl commands passed by the applications are processed by this
 * function accordingly.
 *
 * Input parameters : inode   - Device node holding device information
 *                    fp      - File pointer to the file operations
 *                              structure
 *                    command - (IOCTL) Control command number
 *                    arg     - argument passed to the driver by application
 *
 * Return Value : The return value 0 or positive indicates success
 * and negative value indicates an error. This error number belongs to
 * the error codes defined in <linux/errno.h>
 *
 * Comments : This function is registered into the file_operation structure
 * for processing the device specific control commands. The macros for
 * these commands are defined in the header file "ai64ll_ioctl.h"
 *
 * Revision History :
 * 6-12-01 -  In IOCTL_AI64LL_WRITE_LOCAL_CONFIG_REG the start address of
 *            the register offset is changed to PCI_TO_LOC_0_RANGE_REG.
 * 21-12-01 : IOCTL_AI64LL_READ_REGISTER modified to read all the registers
 *            GSC, PCI and PLX registers.
 * 22-12-01 : Device sampling checks deleted.
 * 02-01-02 : All registers written via IOCTL_AI64LL_WRITE_REGISTER and read via
 *	      IOCTL_AI64LL_READ_REGISTER. Other register read and write IOCTL
 *	      services have been removed.
 * 03-01-02 : Initialize interrupt request notification handling modified in
 *            IOCTL_AI64LL_REQ_INT_NOTIFY ioctl
 */

int
ai64ll_ioctl (struct inode* inode, struct file *fp,
                  unsigned int command, unsigned long arg)
{
	int ai64ll_device_error;
	int scan_reg, ok;
	int inputrange, inputmode, data_format;
	int brdcontrol_val = 0;
    int region;
#ifdef AI64LL_MMAP_SUPPORT	/* drd - MMAP SUPPORT */
	int mmap_reg_select =  0;
#endif /* end AI64LL_MMAP_SUPPORT */
	struct ai64ll_registers registers;
	struct ai64ll_inputscan inputscan;
	ai64ll_int_info_t int_notify;
    unsigned long bindaddr, bindoffset;
  
	struct ai64ll_device *ai64ll_device = 
				(struct ai64ll_device *)fp->private_data;

	DEBUGP1 ("<ioctl> function entry... cmd=0x%x\n",command);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,33)
    if(ai64ll_device->ioctl_processing) {
        DEBUGP0("ioctl processing active: busy\n");
        return (-EBUSY);    /* DO NOT USE IOCTL_RETURN() CALL HERE */
    }
#endif

    ai64ll_device->ioctl_processing++;

	switch (command) {
      
		case IOCTL_AI64LL_NO_COMMAND:
			ai64ll_device->error = AI64LL_SUCCESS;

			DEBUGP0 ("IOCTL_AI64LL_NO_COMMAND entry ... success\n");
            IOCTL_RETURN (0);
      
		/*************** Start of Modification : 21-12-01 *************/
		case IOCTL_AI64LL_READ_REGISTER:

			DEBUGP0 ("IOCTL_AI64LL_READ_REGISTER entry ...\n");
			/* Copy the register structure from the user or application */
			if(copy_from_user (&registers, (void *)arg, sizeof(registers))) {
				ai64ll_device->error = AI64LL_FAULT;
                IOCTL_RETURN (-EFAULT);
            }

			/* Check if the argument passed is valid */
			if(AI64LL_REGISTER_TYPE(registers.ai64ll_register)==AI64LL_GSC_REGISTER ) {
				ok = 0;
				if((registers.ai64ll_register >= AI64LL_GSC_CH(0) )
					&& (registers.ai64ll_register <= 
											AI64LL_GSC_CH(AI64LL_MAX_CHANS)))
					ok = 1;
			
            
				switch(registers.ai64ll_register) {
					case AI64LL_GSC_BCR:
					case AI64LL_GSC_ICR:
					case AI64LL_GSC_CCNT:
					case AI64LL_GSC_SSCR:
					case AI64LL_GSC_FREV:
					case AI64LL_GSC_ACAL:
						ok = 1;
					break;
				}

			
				if(ok) {
					/* Read the register content from the specified register*/
					registers.register_value  = readl (ai64ll_device->local_reg_address +
									AI64LL_REGISTER_INDEX(registers.ai64ll_register));
					DEBUGP1 ("Register value read = %8.8X\n",
							 registers.register_value);
				} 
				else {
					/* Register number that is to be read is invalid */
					ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                    IOCTL_RETURN (-EINVAL);
				}
          
			}
			else 
			if (AI64LL_REGISTER_TYPE(registers.ai64ll_register)==AI64LL_PLX_REGISTER) {
				if ( (registers.ai64ll_register < AI64LL_PLX_LASORR)
						|| (registers.ai64ll_register > AI64LL_PLX_LBRD1)) {
					/* Local config register to be written is Invalid */
					ERRP0("Local config register to be written is Invalid\n");
					ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                    IOCTL_RETURN (-EINVAL);
				}
             
				registers.register_value = readl( PciToLocAddr0Rng +
						AI64LL_REGISTER_INDEX(registers.ai64ll_register));
			}
			else 
			if (AI64LL_REGISTER_TYPE(registers.ai64ll_register)==AI64LL_PCI_REGISTER) {
				if ( (registers.ai64ll_register < AI64LL_PCI_IDR)
					||(registers.ai64ll_register > AI64LL_PCI_ILR_IPR_MGR_MLR)){

					/* Pci config register cannot be written */
					ERRP0("Invalid Pci config register\n");
					ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                    IOCTL_RETURN (-EINVAL);
				}
             
				/*** read the correct configuration location - drd ***/
				pci_read_config_dword ( ai64ll_device->pdev,
					(unsigned long)AI64LL_REGISTER_INDEX(registers.ai64ll_register)*
						(sizeof(u32)), &registers.register_value);
			}
			else {
				ERRP0("Invalid register type\n");
				ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                IOCTL_RETURN (-EINVAL);
			}

       
			/* write the register read content to the structure and
			   copy to user space */
			if(copy_to_user ((void *)arg, &registers, sizeof(registers))) {
				ai64ll_device->error = AI64LL_FAULT;
                IOCTL_RETURN (-EFAULT);
            }

        IOCTL_RETURN (0);

		case IOCTL_AI64LL_WRITE_REGISTER:

			DEBUGP0 ("IOCTL_AI64LL_WRITE_REGISTER entry ...\n");
			/* Copy the register structure from the user/application */
			if(copy_from_user (&registers, (void *)arg, sizeof(registers))) {
				ai64ll_device->error = AI64LL_FAULT;
                IOCTL_RETURN (-EFAULT);
            }
        
			/* Check if the argument passed is valid */
	
			if (AI64LL_REGISTER_TYPE(registers.ai64ll_register)==AI64LL_GSC_REGISTER) {
				goto gsc_register_write;
			}
			else 
			if (AI64LL_REGISTER_TYPE(registers.ai64ll_register)==AI64LL_PCI_REGISTER) {
				goto pci_register_write;
			}
			else 
			if (AI64LL_REGISTER_TYPE(registers.ai64ll_register)==AI64LL_PLX_REGISTER) {
				goto plx_register_write;
			}
			else {
				/* Invalid register type. */
				ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                IOCTL_RETURN (-EINVAL);
	  		}

gsc_register_write:
			switch(registers.ai64ll_register) {
				case AI64LL_GSC_BCR:
				case AI64LL_GSC_ICR:
				case AI64LL_GSC_SSCR:
				case AI64LL_GSC_ACAL:
				break;

				default:
					/* Register number that is to be written is invalid */
					ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                    IOCTL_RETURN (-EINVAL);
				break;
			}

			DEBUGP1 ("REG VALUE : %8.8X\n",
						readl(ai64ll_device->local_reg_address
							+ AI64LL_REGISTER_INDEX(registers.ai64ll_register)));
			/* Write the value to the specified register */
			writel (registers.register_value, ai64ll_device->local_reg_address +
					AI64LL_REGISTER_INDEX(registers.ai64ll_register));
			DEBUGP1 ("REG VALUE after Writing: %8.8X\n", 
					readl(ai64ll_device->local_reg_address
					+ AI64LL_REGISTER_INDEX(registers.ai64ll_register)));
        IOCTL_RETURN (0); /* Return with success */
      
		case IOCTL_AI64LL_REQ_INT_NOTIFY:

        	DEBUGP0 ("IOCTL_AI64LL_REQ_INT_NOTIFY entry ...\n");
        /* Copy the interrupt config structure from the user application */
        	if(copy_from_user (&int_notify, (void *)arg, sizeof(int_notify))) {
				ai64ll_device->error = AI64LL_FAULT;
                IOCTL_RETURN (-EFAULT);
            }

			if ((int_notify.irq0 < IRQ0_NO_INT) 
					||(int_notify.irq0 > IRQ0_AUTOCAL_COMPLETE)) {
				/* Interrupt condition is invalid */
				ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                IOCTL_RETURN (-EINVAL);
			}
			DEBUGP2 ("INT NOTIFY : Int condition : %lu, %8.8lx\n",
						int_notify.init, int_notify.irq0);

			/* Verify that we can make the requested changes. */

			/* Write the interrupt notification mask to the IntCtrlReg
			   for the interrupt condition */
		
			if (int_notify.init)
				ai64ll_device->int_notify.init = 1;
			else
				ai64ll_device->int_notify.init = 0;
		
			if ((int_notify.irq0 >= IRQ0_NO_INT) &&
				(int_notify.irq0 <= IRQ0_AUTOCAL_COMPLETE)) {
				ai64ll_device->int_notify.irq0 = int_notify.irq0;

				DEBUGP1 ("IRQ0 INT NOTIFY : IC : %8.8x\n", readl(IntCtrlReg));
				/* Set the interrupt condition to notify on */

				writel((readl(IntCtrlReg) & (IC_IRQ0_MASK))
					| (int_notify.irq0 << IC_IRQ0_SHIFT), IntCtrlReg);
				DEBUGP1 ("IRQ0 INT NOTIFY after Set: IC : %8.8x\n", 
								readl(IntCtrlReg));
			}
        IOCTL_RETURN (0); /* Return with success */

		case IOCTL_AI64LL_REQ_INT_STATUS:

			/* Report the accumulated interrupt status and clear it. */
			DEBUGP0 ("IOCTL_AI64LL_REQ_INT_STATUS entry ...\n");
			{
				ai64ll_int_info_t	status;
				AI64LL_LOCK();        /* drd */
				memcpy(&status, &ai64ll_device->int_status, sizeof(status));
				memset(	&ai64ll_device->int_status, 0,
								sizeof(ai64ll_device->int_status));
				AI64LL_UNLOCK();       /* drd */
				if(copy_to_user ((void *)arg, (void*) &status, sizeof(status))) {
				    ai64ll_device->error = AI64LL_FAULT;
                    IOCTL_RETURN (-EFAULT);
                }
                IOCTL_RETURN (0); /* Return with success */
	
			}

		case IOCTL_AI64LL_SET_INPUT_MODE:
			DEBUGP0 ("IOCTL_AI64LL_SET_INPUT_MODE entry ...\n");
			/* Get the argument from the user */
			get_user (inputmode, (unsigned long *)arg);

			/* Check if the argument (input mode) passed is valid */
			if ( inputmode < AI64LL_SS_DIFFERENTIAL || inputmode > AI64LL_VREF_TEST ) {
				/* Input mode is invalid */
				ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                IOCTL_RETURN (-EINVAL);
			}

			DEBUGP1 ("I/P MODE : BCR : %8.8x\n", readl(BoardCtrlReg));
			/* Mask the register for Analog input mode bits and Update the
			   BoardCtrlReg with the input mode setting mask */

			writel ( (readl (BoardCtrlReg) & ~BC_AIM_MASK)
					| (inputmode << BC_AIM_SHIFT), BoardCtrlReg );

			DEBUGP1("I/P MODE after SET : BCR : %8.8x\n", 
												readl(BoardCtrlReg));
        IOCTL_RETURN (0); /* Return with success */

		case IOCTL_AI64LL_SET_RANGE:
			/* Get the argument from the user */
			get_user ( inputrange, (unsigned long *)arg);

			/* Check if the argument (input range) passed is valid */
			if ( inputrange < AI64LL_INPUT_RANGE_2_5 || inputrange > AI64LL_INPUT_RANGE_10 ) {
				/* Input range is  invalid */
				ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                IOCTL_RETURN (-EINVAL);
			}

			DEBUGP1 ("I/P RANGE : BCR : %8.8x\n", readl(BoardCtrlReg));
        
			/* Mask the register for Analog input range bits and Update
			   the BoardCtrlReg with the input range setting mask */

			writel ( (readl (BoardCtrlReg) & ~BC_RANGE_MASK)
					| (inputrange << BC_RANGE_SHIFT), BoardCtrlReg);

			DEBUGP1 ("I/P RANGE after SET: BCR : %8.8x\n", 
						readl(BoardCtrlReg));
        IOCTL_RETURN (0); /* Return with success */

		case IOCTL_AI64LL_SET_DATA_FORMAT:
			/* Get the argument from the user */
			get_user ( data_format, (unsigned long *)arg);
        
			/* Check if the argument (data_format) passed is valid */
			if (data_format < AI64LL_FORMAT_TWO_COMPLEMENT
							|| data_format > AI64LL_FORMAT_OFFSET_BINARY ) {
				/* Data format is  invalid */
				ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                IOCTL_RETURN (-EINVAL);
			}

			DEBUGP1 ("DATA FORMAT : BCR : %8.8x\n", readl(BoardCtrlReg));
			/* Mask the register for Data format bits and Update the
			   BoardCtrlReg with the data format setting mask */

			writel ((readl (BoardCtrlReg) & ~BC_DATA_FORMAT_MASK)
					| (data_format << BC_DATA_FORMAT_SHIFT),BoardCtrlReg);

			DEBUGP1 ("DATA FORMAT after SET: BCR : %8.8x\n", 
					readl(BoardCtrlReg));
            IOCTL_RETURN (0); /* Return with success */

		case IOCTL_AI64LL_AUTO_CAL:
			DEBUGP0 ("IOCTL_AI64LL_AUTO_CAL entry ...\n");
			ai64ll_device->autocalibration = 1;
            ai64ll_device->timeout_retry_count = (AUTO_CAL_TIMEOUT_MS/10);

			/* enable autocalibration interrupts */
			writel((readl(IntCtrlReg) & (IC_IRQ0_MASK))
				| (IC_AUTO_CAL_COMP_VAL << IC_IRQ0_SHIFT), IntCtrlReg);
			DEBUGP1 ("IntCtrlReg : %8.8x\n",readl(IntCtrlReg));
       
			brdcontrol_val =  readl (BoardCtrlReg);
			DEBUGP1 ("BCR : %8.8x\n",brdcontrol_val);
			/* Set the Auto calibration bit in the Board Control register */
			brdcontrol_val |= BC_AUTO_CAL_MASK;

			DEBUGP1 ("BCR MASK written : %8.8x\n",brdcontrol_val);
			writel (brdcontrol_val, BoardCtrlReg);
			/* sleep_on_timeout () for this task for a time of AUTO_CAL_TIMEOUT_MS */

            while(ai64ll_device->timeout_retry_count--) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
                            wait_event_interruptible_timeout(ai64ll_device->ioctlwq,
                                                             0,
                                                             MSECS_TO_SLEEP(10));
#else
			    interruptible_sleep_on_timeout(&ai64ll_device->ioctlwq,
							MSECS_TO_SLEEP(10)); /* sleep for 10 ms */
#endif
                if(ai64ll_device->autocalibration == 0)
                    break;

                /* check if autocal complete */
			    brdcontrol_val =  readl (BoardCtrlReg);
                if((brdcontrol_val & BC_AUTO_CAL_MASK) == 0) {
                    ai64ll_device->autocalibration = 0;   /* mark as done */
                    break;
                }
            }

			DEBUGP1 ("BCR after CLR: %8.8x\n",readl(BoardCtrlReg));

			/* If interrupt notification is requested, check the timeout */
			if ( ai64ll_device->autocalibration == 1 ) { /* Timeout occurred */
				ai64ll_device->error = AI64LL_IOCTL_TIMEOUT;
                IOCTL_RETURN (-EIO);
			}
			else
                IOCTL_RETURN (0); /* Return with success */
        
		break;

		case IOCTL_AI64LL_INITIALIZE:
			DEBUGP0 ("IOCTL_AI64LL_INITIALIZE entry ...\n");
			ai64ll_device->initialize = 1;
            ai64ll_device->timeout_retry_count = INITIALIZE_TIMEOUT_MS;
			DEBUGP1 ("INITIALIZE : BCR : %8.8x\n", readl(BoardCtrlReg));

			brdcontrol_val =  readl (BoardCtrlReg);
       
			/* Set the Auto calibration bit in the Board Control register */
			brdcontrol_val |= BC_INITIALIZE_MASK;
			writel (brdcontrol_val, BoardCtrlReg);
       
            while(ai64ll_device->timeout_retry_count--) {
			    /* sleep_on_timeout() for this task for a time of INITIALIZE_TIMEOUT_MS*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
                            wait_event_interruptible_timeout(ai64ll_device->ioctlwq,
                                                             0,
                                                             MSECS_TO_SLEEP(1));
#else
			    interruptible_sleep_on_timeout(&ai64ll_device->ioctlwq,
							MSECS_TO_SLEEP(1)); /* sleep for 1 ms */
#endif

                if(ai64ll_device->initialize == 0)
                    break;

                /* check to see if Initialization is complete */
			    brdcontrol_val =  readl (BoardCtrlReg);
                if((brdcontrol_val & BC_INITIALIZE_MASK) == 0) {
                    ai64ll_device->initialize = 0;   /* mark as done */
                    break;
                }
            }

		    if ( ai64ll_device->initialize == 1 ) { /* Timeout has occurred */
                ai64ll_device->error = AI64LL_IOCTL_TIMEOUT;
                ERRP0 ("Initialize Timeout occurred\n");
                IOCTL_RETURN (-EIO);
            }
            else
                IOCTL_RETURN (0); /* Return with success */
		break;

		case IOCTL_AI64LL_SET_INPUT_SCAN:
			/* Copy the input scan structure from the user/application */
			if(copy_from_user (&inputscan, (void *)arg, sizeof(inputscan))) {
				ai64ll_device->error = AI64LL_FAULT;
                IOCTL_RETURN (-EFAULT);
            }

			/* Check if the scan size, scan clock source, scan mode and
			   scan channel arguments are valid */

			if ( (inputscan.scan_size < SCAN_SIZE_1_CHAN
					|| inputscan.scan_size > SCAN_SIZE_64_CHAN)
					|| ( inputscan.enable_scan & ~1)) {
				/* Input scan arguments are invalid */
				ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                IOCTL_RETURN (-EINVAL);
			}

			if ((inputscan.scan_size == SCAN_SIZE_1_CHAN) && 
				((inputscan.single_scan_channel < MIN_CHANNEL) ||
					(inputscan.single_scan_channel > MAX_CHANNEL))) {
				/* Input scan arguments are invalid */
				ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                IOCTL_RETURN (-EINVAL);
			}
		
			DEBUGP1 ("INPUT SCAN : SCAN & SYNC REG : %8.8x\n",
										readl (ScanSyncCtrlReg));
			/* Read the scan register*/
			scan_reg = readl(ScanSyncCtrlReg);

			/* Mask off bits to be set. */
			scan_reg = scan_reg & (~SC_AIN_SCAN_SIZE_MASK)
							& (~ SC_AIN_ENABLE_SCAN_MASK);

			/* Set the bits as per new selections */

			scan_reg |=  (inputscan.scan_size << SC_AIN_SCAN_SIZE_SHIFT)
					| (inputscan.enable_scan << SC_AIN_ENABLE_SCAN_SHIFT);

			/* if single channel scan, use new channel */
			if(inputscan.scan_size == SCAN_SIZE_1_CHAN) {
					scan_reg = scan_reg & (~ SC_AIN_SINGLE_CHAN_MASK);
					scan_reg |= (inputscan.single_scan_channel
								<< SC_AIN_SINGLE_CHAN_SHIFT);
			}

			/* Write to the Scan Control Register. */
			writel (scan_reg, ScanSyncCtrlReg);
			DEBUGP1 ("INPUT SCAN after Set: SCAN & SYNC REG : %8.8x\n",
                                           readl (ScanSyncCtrlReg));
        IOCTL_RETURN (0); /* Return with success */

		case IOCTL_AI64LL_GET_DEVICE_ERROR:
			DEBUGP0 ("IOCTL_AI64LL_GET_DEVICE_ERROR entry ...\n");
			/* Get the error that has occurred for the last call to the
			   driver from the ai64ll_device structure */
      
			ai64ll_device_error = ai64ll_device->error;
      
			/* Copy the error to the user that is updated in the device structure */
			put_user (ai64ll_device_error, (unsigned long *)arg);
        IOCTL_RETURN (0); /* Return with success */

pci_register_write:

		DEBUGP0 ("pci_register_write entry ...\n");
		if ( (registers.ai64ll_register < AI64LL_PCI_CR_SR)
			|| (registers.ai64ll_register > AI64LL_PCI_ILR_IPR_MGR_MLR)) {
          
			/* Pci config register cannot be written */
			ERRP0 ("Pci config register cannot be written");
			ai64ll_device->error = AI64LL_INVALID_PARAMETER;
            IOCTL_RETURN (-EINVAL);
		}
      
		if ( (registers.ai64ll_register > AI64LL_PCI_CR_SR)
			&& (registers.ai64ll_register < AI64LL_PCI_CLSR_LTR_HTR_BISTR) ) {
			/* Class revision id register cannot be written -- Invalid */
			ERRP0("Class revision id register cannot be written -- Invalid\n");

			ai64ll_device->error = AI64LL_INVALID_PARAMETER;
            IOCTL_RETURN (-EINVAL);
		}
      
		if ( (registers.ai64ll_register > AI64LL_PCI_BAR3)
			&& (registers.ai64ll_register < AI64LL_PCI_ERBAR)) {
			/* Subsystem id register cannot be written -- invalid */
			ERRP0 ("Subsystem id register cannot be written -- invalid\n");
			ai64ll_device->error = AI64LL_INVALID_PARAMETER;
            IOCTL_RETURN (-EINVAL);
		}

		if ( (registers.ai64ll_register > AI64LL_PCI_ERBAR)
			&& (registers.ai64ll_register < AI64LL_PCI_ILR_IPR_MGR_MLR)) {
			/* Register 13 and 14 cannot be written */
			ERRP0 ("Register 13 and 14 cannot be written\n");
			ai64ll_device->error = AI64LL_INVALID_PARAMETER;
            IOCTL_RETURN (-EINVAL);
		}

      
		/* Write the value to the requested register offset */
		/*** write to the correct configuration register - drd ***/
		pci_write_config_dword (ai64ll_device->pdev,
		AI64LL_REGISTER_INDEX(registers.ai64ll_register) * (sizeof(unsigned int)),
					registers.register_value);
      
        IOCTL_RETURN (0); /* Return with success */

plx_register_write:

		DEBUGP0 ("plx_register_write entry ...\n");

		if ((registers.ai64ll_register < AI64LL_PLX_LASORR)
				|| (registers.ai64ll_register > AI64LL_PLX_LBRD1)) {
			/* Local config register to be written is Invalid */
			ERRP0("Local config register to be written is Invalid\n");
			ai64ll_device->error = AI64LL_INVALID_PARAMETER;
            IOCTL_RETURN (-EINVAL);
		}
      
		/* Write to the value to the register offset */
		writel (registers.register_value, (PciToLocAddr0Rng
					+ AI64LL_REGISTER_INDEX(registers.ai64ll_register)) );
        IOCTL_RETURN (0); /* Return with success */

		case IOCTL_AI64LL_READ_DEBUG:

			DEBUGP0 ("IOCTL_AI64LL_READ_DEBUG ...entry\n");
			/* Copy the debug messages from the device structure's buffer
			   to the user */
      
			if(copy_to_user ((void *)arg, ai64ll_device->debug_buffer,
					sizeof(ai64ll_device->debug_buffer))) {
				ai64ll_device->error = AI64LL_FAULT;
                IOCTL_RETURN (-EFAULT);
            }
        IOCTL_RETURN (0); /* Return with success */

		case IOCTL_AI64LL_GET_DRIVER_INFO:

			DEBUGP0 ("IOCTL_AI64LL_GET_DRIVER_INFO ...entry\n");
			{
				ai64ll_driver_info_t	info;
				memset(&info, 0, sizeof(info));
				strcpy(info.version, AI64LL_VERSION);
				strcpy(info.built, __DATE__ ", " __TIME__);

				/* Copy the structure to the user */
				if(copy_to_user((void*) arg, &info, sizeof(info))) {
				    ai64ll_device->error = AI64LL_FAULT;
                    IOCTL_RETURN (-EFAULT);
                }
                IOCTL_RETURN (0); /* Return with success */
			}

#ifdef AI64LL_MMAP_SUPPORT	/* drd - MMAP SUPPORT */
        case IOCTL_AI64LL_MMAP_GET_OFFSET: /* NEW WAY - use this ioctl */
			DEBUGP0 ("IOCTL_AI64LL_MMAP_GET_OFFSET ...entry\n");

            /* Get the argument from the user */
            get_user(mmap_reg_select, (unsigned long *) arg);

			/* Check if the argument passed is valid */
			if ( (mmap_reg_select != AI64LL_SELECT_GSC_MMAP) && 
                               ( mmap_reg_select != AI64LL_SELECT_PLX_MMAP ))
			{
				/* Invalid argument */
				ai64ll_device->error = AI64LL_INVALID_PARAMETER;
                IOCTL_RETURN (-EINVAL);
			}

            ai64ll_device->mmap_reg_select = mmap_reg_select;

            region = (mmap_reg_select == AI64LL_SELECT_GSC_MMAP) ?
                LOCAL_REGION : CONFIG_REGION;

		    bindaddr = (ai64ll_device->mem_region[region].address/PAGE_SIZE) * 
								PAGE_SIZE;
		    bindoffset = ai64ll_device->mem_region[region].address - bindaddr;

            if(copy_to_user((void *) arg, &bindoffset, sizeof(unsigned long))) {
                IOCTL_RETURN (-EFAULT);
            }

        IOCTL_RETURN (0); /* Return with success */

#endif /* end AI64LL_MMAP_SUPPORT */
     
		default :
			DEBUGP1 ("Invalid ioctl command 0x%x\n",command);
			/* Invalid ioctl command */
        IOCTL_RETURN (-EINVAL);
	}
}

static int
fasync_ai64ll (int fd , struct file *fp, int on)
{
	int retval;

	DEBUGP0 ("<fasync_ai64ll> function ...entry\n");
     
	/* Call the kernel fasync_helper function for the ai64ll_fasyncptr */
	retval = fasync_helper (fd, fp, on, &ai64ll_fasyncptr);

	if (retval < 0)
		return retval;         /* Failure, return error */
	return 0;               /* Success, return success */
}

/*
 * File : ai64ll_irq_handler()
 *
 * Description : This is an Interrupt service routine for the AI64LL
 * device. This routine services the various interrupt requests for the
 * device and does the required operation.
 *
 * Input parameters : irq    - IRQ line number allocated to the device
 *                    dev_id - Device id (structure holding device info.)
 *
 * Return Value : None
 *
 * Comments : This routine is registered in the open () module for the
 * irq line allocated to the device. Whenever an interrupt occurs
 * on this device this function gets invoked.
 *
 * Revision History : kill_fasync() modified for 2.4 kernel.
 * 22-12-01: Interrupt notification request check added before notifying user.
 *
 */

static irqreturn_t
ai64ll_irq_handler (int irq, void * dev_id)
{
	int int_condition;
	int int_csr;			/* int csr - drd */
	int dma_csr;          /* dma cse - drd */
	int irq0_condition = 0;
  
	/* Get the device structure for the particular device / board */
	struct ai64ll_device *ai64ll_device = ( struct ai64ll_device *) dev_id;

	AI64LL_LOCK();   /*** set the lock - drd ***/

	DEBUGP0(":: ISR :: Interrupt Service Routine ...entry\n");
	DEBUGP3(":: ISR :: DMA_ST=0x%x INTCTRL=0x%x INTCSR=0x%x\n", 
				readl(DmaCmdStatus),readl(IntCtrlReg),
				readl(IntCtrlStatus));
   
	int_csr = readl(IntCtrlStatus);	/* read intint_csr -drd */
 
	/*** drd ***/
	/*** DO NOT USE DMA_INPROGRESS FLAG. USE THE IntCtrlStatus DMA BIT 
	 *** INSTEAD
	 ***/
  
	if ( int_csr & 0x00200000 ) { /* DMA 0 interrupt occurred??? */
		DEBUGP0 (":: ISR :: DMA interrupt occurred [SHOULD NOT OCCUR!!!]\n");

		/* Check if the DMA_CMD_STAT_DONE flag is set in the
		   DmaCmdStatus Register */
      
		if ( readl(DmaCmdStatus) & DMA_CMD_STAT_DONE ) {
			/*Clear the DMA interrupt by clearing DMA Command Status Register */
			/*** drd ***/
			/*** clear dma interrupts ***/
			dma_csr =readl(DmaCmdStatus) & 
						STOP_DMA_CMD_0_MASK & ~DMA_0_CSR_ENABLE;
	
			/* Clear the DMA Command status register */
			writel (dma_csr, DmaCmdStatus);
          
			/* Clear interrupts */
			writel ((dma_csr | DMA_0_CSR_CLR_INT), DmaCmdStatus);
			goto ENABLE_IRQ;
		}
	}
	else  {  /* It is a local interrupt */
		DEBUGP0 (":: ISR :: Local interrupt occurred\n");
		/* Read the IntCtrlReg to find the interrupt condition */
		int_condition = readl(IntCtrlReg) & IC_INT_OCC_MASK;

		if ( readl(IntCtrlReg) & IC_IRQ0_REQUEST_MASK) {
			DEBUGP0 (":: ISR :: IRQ0 interrupt occurred\n");
			/* Clear the interrupt request by clearing in the Interrupt
               ControlRegister*/

			writel((readl(IntCtrlReg) & ~IC_IRQ0_REQUEST_MASK), IntCtrlReg);
			irq0_condition = readl(IntCtrlReg);
			/* Identify the interrupt condition */
			if ( irq0_condition == IC_IDLE_VAL ) {
				DEBUGP0 (":: ISR :: INITIALIZE interrupt occurred\n");
				/* Set the aux input low to high flag in the ai64ll device
                   structure */
                
				/* only ok if initialization complete */
                if((readl (BoardCtrlReg) & BC_INITIALIZE_MASK) == 0) {
				    ai64ll_device->initialize = 0;
                }

				/* Wake up the process waiting on the ioctl queue */
				wake_up_interruptible (&ai64ll_device->ioctlwq);

				/** Modification: Interrupt request check: 21-12-01 **/
				if ( ai64ll_device->int_notify.init ) {
					/* Signal the application for the interrupt notification
					   condition */
		    
					ai64ll_device->int_status.init++;
					if (ai64ll_fasyncptr) {
#ifdef LINUX_2_4
						kill_fasync (&ai64ll_fasyncptr, SIGIO, POLL_IN);
#else /* else !LINUX_2_4 */
						kill_fasync (ai64ll_fasyncptr, SIGIO, POLL_IN);
#endif /* end LINUX_2_4 */
					}
				}
				goto ENABLE_IRQ;
			}
            
			/* Auto calibration Interrupt condition */
			if ( irq0_condition == IC_AUTO_CAL_COMP_VAL ) {
				DEBUGP0 (":: ISR :: AUTO CAL interrupt occurred\n");
				/* Clear the auto calibration flag in the ai64ll device
				   structure */

				/* only ok if autocal complete */
                if((readl (BoardCtrlReg) & BC_AUTO_CAL_MASK) == 0) {
				    ai64ll_device->autocalibration = 0;
                }

				/* wake up the process waiting on the ioctl queue */
				wake_up_interruptible (&ai64ll_device->ioctlwq);

				/********* Start of Modification : 22-12-01 ********/
				if ( ai64ll_device->int_notify.irq0 == IRQ0_AUTOCAL_COMPLETE ) {
					/* Signal the application for the interrupt notification
					   condition */

					ai64ll_device->int_status.irq0++;
                     
					if (ai64ll_fasyncptr)
#ifdef LINUX_2_4
						kill_fasync (&ai64ll_fasyncptr, SIGIO, POLL_IN);
#else /* else !LINUX_2_4 */
						kill_fasync (ai64ll_fasyncptr, SIGIO, POLL_IN);
#endif /* end LINUX_2_4 */
				}
				goto ENABLE_IRQ;
			}
		} /* end of irq0 */

		if ( readl(IntCtrlReg) & IC_IRQ1_REQUEST_MASK) {
			DEBUGP0 (":: ISR :: IRQ1 interrupt occurred "
									"[SHOULD NOT OCCUR!!!]\n");
			/* Clear the interrupt request by clearing in the Interrupt control
			   Register*/
            
			writel((readl(IntCtrlReg) & ~IC_IRQ1_REQUEST_MASK), IntCtrlReg);
			goto ENABLE_IRQ;
		} /* end of irq1 */
	}

ENABLE_IRQ :
	AI64LL_UNLOCK();       /*** unlock - drd ***/
	return IRQ_HANDLED;
}

/********************************************************************
 *                                                                  *
 *      mmap() support                                              *
 *                                                                  *
 ********************************************************************/
#ifdef AI64LL_MMAP_SUPPORT	/* drd - MMAP SUPPORT */
static int ai64ll_mmap(struct file *fp, struct vm_area_struct *vma)
{
	struct ai64ll_device *ai64ll_device;
	u32 size, page, round_size;
	int	ret=0;

	/* Get the device structure from the private_data field */
	ai64ll_device	= (struct ai64ll_device *)fp->private_data;
	size = vma->vm_end - vma->vm_start;

	DEBUGP2 ("mmap(): physical local register: address=0x%x size=0x%x\n",
			ai64ll_device->mem_region[LOCAL_REGION].address,
			ai64ll_device->mem_region[LOCAL_REGION].size);
	DEBUGP2 ("mmap(): physical config register: address=0x%x size=0x%x\n",
			ai64ll_device->mem_region[CONFIG_REGION].address,
			ai64ll_device->mem_region[CONFIG_REGION].size);

	DEBUGP2  ("mmap(): start=0x%x end=0x%x\n",
		(int)vma->vm_start,(int)vma->vm_end);

	/* Use mmap_reg_select to distinguish between Control/DataGSC  register 
     *  and PLX register */
	switch((int)ai64ll_device->mmap_reg_select) {	
	case AI64LL_SELECT_GSC_MMAP:/* Control/Data - AI64LL (GSC) Registers */
		round_size = ((ai64ll_device->mem_region[LOCAL_REGION].size + 
							PAGE_SIZE-1)/PAGE_SIZE) * PAGE_SIZE;
						
		DEBUGP2("Input Size = 0x%x Local Page Rounded Size = 0x%x\n",
												(u32)size,(u32)round_size);

		if (size > round_size) {
			ret = -EINVAL;
		}

#if 0
		bindaddr=(ai64ll_device->mem_region[LOCAL_REGION].address/PAGE_SIZE) * 
								PAGE_SIZE;
		bindoffset = ai64ll_device->mem_region[LOCAL_REGION].address - bindaddr;
		vma->vm_start += bindoffset;
#endif
		page = ai64ll_device->mem_region[LOCAL_REGION].address;
        vma->vm_flags |= VM_IO; /* NEEDED IF USER DOES AN mlockall() */

		DEBUGP2 ("mmap(): GSC vm_start=0x%x page=0x%x\n",
			(int)vma->vm_start,(int)page);
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,9)
		if (remap_page_range(vma, vma->vm_start, page, size, vma->vm_page_prot))
		{
		ret = -EAGAIN;
		}
#else
		if (remap_pfn_range(vma, vma->vm_start, (page >> PAGE_SHIFT), size, vma->vm_page_prot))
		{
		ret = -EAGAIN;
		}
#endif

	break;

	case AI64LL_SELECT_PLX_MMAP:	/* PLX Registers */
		round_size = ((ai64ll_device->mem_region[CONFIG_REGION].size + 
							PAGE_SIZE-1)/PAGE_SIZE) * PAGE_SIZE;
		DEBUGP2("Input Size = 0x%x Local Page Rounded Size = 0x%x\n",
												(u32)size,(u32)round_size);
		if (size > round_size) {
			ret = -EINVAL;
		}

#if 0
		bindaddr=(ai64ll_device->mem_region[CONFIG_REGION].address/PAGE_SIZE) *
								PAGE_SIZE;
		bindoffset = ai64ll_device->mem_region[CONFIG_REGION].address-bindaddr;
		vma->vm_start += bindoffset;
#endif
		page = ai64ll_device->mem_region[CONFIG_REGION].address;

        vma->vm_flags |= VM_IO; /* NEEDED IF USER DOES AN mlockall() */

		DEBUGP2 ("mmap(): PLX vm_start=0x%x page=0x%x\n",
			(int)vma->vm_start,(int)page);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,9)
		if (remap_page_range(vma, vma->vm_start, page, size, vma->vm_page_prot))
		{
		ret = -EAGAIN;
		}
#else
		if (remap_pfn_range(vma, vma->vm_start, (page >> PAGE_SHIFT), size, vma->vm_page_prot))
		{
		ret = -EAGAIN;
		}
#endif

	break;
	default:
		ret = -EINVAL;
	break;
	}

	return ret;
}
#endif	/* end AI64LL_MMAP_SUPPORT */

/********************************************************************
 *                                                                  *
 *      Configure memory regions                                    *
 *                                                                  *
 ********************************************************************/
int
ai64ll_configure_mem_regions(struct pci_dev *pdev, int which, 
					u32 *address, u32 *size, u32 *flags)
{
	/*** First get the requested memory region ***/
	*address = 0;
	*address = pci_resource_start(pdev,which);
	*size	= pci_resource_end(pdev,which) - *address + 1;
	DEBUGP3 ("PCI_BASE_ADDRESS_%d: address = 0x%x, size = 0x%x\n",
				which, *address, *size);

	/*** Next, request the memory regions so that they are marked busy ***/

	*flags = pdev->resource[which].flags;

	/* I/O Region */
	if(pdev->resource[which].flags & IORESOURCE_IO) {
			DEBUGP3 ("Device %x Vendor %x:  region %d is I/O mapped\n",
					pdev->device,pdev->vendor,which);

		if(!request_region(*address, *size, MODULE_NAME)) {
			DEBUGP3 ("Device %x Vendor %x:  I/O region %d is busy\n",
						pdev->device,pdev->vendor,which);
			*address = 0;	/* do not release it */
			return(1);
		}

	} else { /* Memory Region */

			DEBUGP3 ("Device %x Vendor %x:  region %d is memory mapped\n",
					pdev->device,pdev->vendor,which);

		if(!request_mem_region(*address, *size, MODULE_NAME)) {
			DEBUGP3 ("Device %x Vendor %x:  memory region %d is busy\n",
						pdev->device,pdev->vendor,which);
			*address = 0;	/* do not release it */
			return(1);
		}
	}
	return(0);
}

/********************************************************************
 *                                                                  *
 *      De-configure memory regions                                 *
 *                                                                  *
 ********************************************************************/
void
ai64ll_deconfigure_mem_regions(int which, u32 address, u32 size, u32 flags)
{
	DEBUGP3 ("ai64ll_deconfigure_mem_regions = 0x%x " 
				"size=0x%x flags=0x%x\n", address, size, flags);

	/* if no address to deconfigure, simply return */
	if(!address)
		return;

	/* I/O Region */
	if(flags & IORESOURCE_IO) {
			DEBUGP3 ("Address %x Size %x:  Deconfigure I/O region %d\n",
					address, size, which);

		release_region(address, size);

	} else { /* Memory Region */
			DEBUGP3 ("Address %x Size %x:  Deconfigure memory region %d\n",
					address, size, which);

		release_mem_region(address, size);
	}
}

module_init(ai64ll_init);
module_exit(ai64ll_exit);
