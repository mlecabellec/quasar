// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_kernel_4.h $
// $Rev: 52470 $
// $Date: 2023-02-13 09:58:00 -0600 (Mon, 13 Feb 2023) $

// Device Driver: Linux: header file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#ifndef	__OS_KERNEL_4_H__
#define	__OS_KERNEL_4_H__

#include <linux/version.h>

#if	(LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)) && \
	(LINUX_VERSION_CODE <  KERNEL_VERSION(5,0,0))

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/page.h>
#include <asm/types.h>

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/wait.h>

#if	LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif



// macros *********************************************************************

#define	os_bar_pci_size(d,b)		pci_resource_len((d)->pci.pd,(b))

#define	EVENT_RESUME_IRQ(q,c)		c = 1; if (q) wake_up_interruptible(q)	// occurs inside ISR
#define	EVENT_WAIT_IRQ(q,c)			wait_event_interruptible((q)[0],(c))
#define	EVENT_WAIT_IRQ_TO(q,c,t)	wait_event_interruptible_timeout((q)[0],(c),(t))
#define	FALLTHROUGH					// for the 5.9 kernel's "fallthrough" macro
#define	FASYNC_SET(_ptr,_func)		(_ptr)->fasync = (_func)
#define	GSC_HAVE_IOCTL_BKL			0
#define	IOCTL_SET_BKL(_ptr,_func)
#define	KILL_FASYNC(q,s,b)			kill_fasync(&(q),(s),(b))		// SIO4 specific
#define	MEM_ACCESS_OK(t,p,s)		access_ok((t),(p),(s))			// may be redefined below for RHEL
#define	MEM_ALLOC_LIMIT				(2L * 1024L * 1024)
#undef	MOD_DEC_USE_COUNT
#define	MOD_DEC_USE_COUNT
#undef	MOD_INC_USE_COUNT
#define	MOD_INC_USE_COUNT
#define	OS_MSI_ENABLE(d)			pci_enable_msi((d)->pci.pd)
#define	OS_MSI_DISABLE(d)			pci_disable_msi((d)->pci.pd)
#define	OS_SUPPORTS_MSI				1
#define	PAGE_RESERVE(vpa)			SetPageReserved(virt_to_page((vpa)))
#define	PAGE_UNRESERVE(vpa)			ClearPageReserved(virt_to_page((vpa)))
#define	PCI_CLEAR_MASTER(p)			pci_clear_master((p))

#define	PCI_DEVICE_LOOP(p)	\
	for ((p) = NULL; ((p) = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, (p))); )

#define	PCI_DEVICE_SUB_SYSTEM(p)	((p)->subsystem_device)
#define	PCI_DEVICE_SUB_VENDOR(p)	((p)->subsystem_vendor)
#define	PCI_DISABLE_DEVICE(p)		pci_disable_device((p))
#define	PCI_ENABLE_DEVICE(p)		pci_enable_device((p))
#define	PCI_SET_DMA_MASK(dev,mask)	pci_set_dma_mask((dev)->pci.pd,(dma_addr_t)(mask))
#define	PROC_GET_INFO(ptr,func)
#define	PROC_GET_INFO_SUPPORTED		0
#define	REGION_IO_CHECK(a,s)		0
#define	REGION_IO_RELEASE(a,s)		release_region(a,s)
#define	REGION_IO_REQUEST(a,s,n)	request_region(a,s,n)
#define	REGION_MEM_CHECK(a,s)		0
#define	REGION_MEM_RELEASE(a,s)		release_mem_region(a,s)
#define	REGION_MEM_REQUEST(a,s,n)	request_mem_region(a,s,n)
#define	REGION_TYPE_IO_BIT			IORESOURCE_IO
#define	SET_CURRENT_STATE(s)		__set_current_state(s)
#define	SET_MODULE_OWNER(p)
#define	VADDR_T						void*
#define	WAIT_QUEUE_ENTRY_INIT(w,c)	init_waitqueue_entry((w),(c))
#define	WAIT_QUEUE_HEAD_INIT(q)		init_waitqueue_head((q))

#define	IRQ_REQUEST(dv,fnc)			request_irq((dv)->pci.pd->irq,			\
												(fnc),						\
												(unsigned long) IRQF_SHARED,\
												DEV_NAME,					\
												(dv))

#define	IRQ_REQUEST_MSI(dv,fnc)		request_irq((dv)->pci.pd->irq,	\
												(fnc),				\
												0,					\
												DEV_NAME,			\
												(dv))

#if HAVE_COMPAT_IOCTL
	#define	IOCTL_SET_COMPAT(_ptr,_func)	(_ptr)->compat_ioctl = (_func)
#else
	#define	IOCTL_SET_COMPAT(_ptr,_func)
	#include "linux/ioctl32.h"
#endif

#if HAVE_UNLOCKED_IOCTL
	#define	IOCTL_SET_UNLOCKED(_ptr,_func)	(_ptr)->unlocked_ioctl = (_func)
#else
	#define	IOCTL_SET_UNLOCKED(_ptr,_func)
#endif

#if (BITS_PER_LONG == 32)
	#define	IOCTL_32BIT_SUPPORT				GSC_IOCTL_32BIT_NATIVE
#elif (HAVE_COMPAT_IOCTL)		// 64-bit OS
	#define	IOCTL_32BIT_SUPPORT				GSC_IOCTL_32BIT_COMPAT
#elif defined(CONFIG_COMPAT)	// !HAVE_COMPAT_IOCTL, 64-bit OS
	#define	IOCTL_32BIT_SUPPORT				GSC_IOCTL_32BIT_TRANSLATE
#else // (!HAVE_COMPAT_IOCTL)	// 64-bit OS
	#define	IOCTL_32BIT_SUPPORT				GSC_IOCTL_32BIT_NONE
#endif

#ifdef GFP_DMA32
	#define	GFP_DMA_GSC						GFP_DMA32
#else
	#define	GFP_DMA_GSC						GFP_DMA
#endif

#if	LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
	#define	WAIT_QUEUE_ENTRY_T				struct wait_queue_entry
	#define	WAIT_QUEUE_HEAD_T				struct wait_queue_head
#else
	#define	WAIT_QUEUE_ENTRY_T				wait_queue_t
	#define	WAIT_QUEUE_HEAD_T				wait_queue_head_t
#endif

#if (defined(RHEL_RELEASE_CODE)) && (defined(RHEL_RELEASE_VERSION))
	#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(8,1)
		#undef	MEM_ACCESS_OK
		#define	MEM_ACCESS_OK(t,p,s)		access_ok((p),(s))
	#endif
#endif



// data types *****************************************************************

typedef struct timeval	os_time_t;



// prototypes *****************************************************************

irqreturn_t	os_irq_isr(int irq, void* dev_id);
int			os_proc_get_info(char* page, char** start, off_t offset, int count);



#endif
#endif
