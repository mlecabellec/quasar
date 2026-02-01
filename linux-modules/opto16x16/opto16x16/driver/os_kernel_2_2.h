// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_kernel_2_2.h $
// $Rev: 52470 $
// $Date: 2023-02-13 09:58:00 -0600 (Mon, 13 Feb 2023) $

// Device Driver: Linux: header file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#ifndef	__OS_KERNEL_2_2_H__
#define	__OS_KERNEL_2_2_H__

#include <linux/version.h>

#if	(LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)) && \
	(LINUX_VERSION_CODE <  KERNEL_VERSION(2,3,0))

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/page.h>
#include <asm/spinlock.h>
#include <asm/types.h>
#include <asm/uaccess.h>

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/malloc.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/wrapper.h>



// macros *********************************************************************

#define	os_bar_pci_size(d,b)		(os_bar_size((d),(b)))

#define	PCI_CLEAR_MASTER(p)

#define	del_timer_sync(t)			del_timer((t))
#define	min(a,b)					(((a) < (b)) ? (a) : (b))

#define	__GFP_NOWARN				0
#define	EVENT_RESUME_IRQ(q,c)		if (q) wake_up_interruptible(q)		// occurs inside ISR
#define	EVENT_WAIT_IRQ(q,c)			interruptible_sleep_on(q)
#define	EVENT_WAIT_IRQ_TO(q,c,t)	interruptible_sleep_on_timeout(q,t)
#define	FALLTHROUGH					// for the 5.9 kernel's "fallthrough" macro
#define	FASYNC_SET(_ptr,_func)		(_ptr)->fasync = (_func)
#define	GSC_HAVE_IOCTL_BKL			1	// Big Kernel Lock based ioctl()
#define	IOCTL_32BIT_SUPPORT			GSC_IOCTL_32BIT_NATIVE
#define	IOCTL_SET_BKL(_ptr,_func)	(_ptr)->ioctl = (_func)
#define	IOCTL_SET_COMPAT(p,f)
#define	IOCTL_SET_UNLOCKED(p,f)
#define	KILL_FASYNC(q,s,b)			kill_fasync((q),(s),(b))		// SIO4 specific
#define	MEM_ACCESS_OK(t,p,s)		access_ok((t),(p),(s))
#define	MEM_ALLOC_LIMIT				(2L * 1024L * 1024)
#define	MEM_VIRT_TO_DMA_ADRS(p)		virt_to_bus(p)
#define	OS_SUPPORTS_MSI				0
#define	PAGE_RESERVE(vpa)			mem_map_reserve(MAP_NR((vpa)))
#define	PAGE_UNRESERVE(vpa)			mem_map_unreserve(MAP_NR((vpa)))
#define	PCI_DEVICE_LOOP(p)			for (p=pci_devices;p;p=p->next)
#define	PCI_DEVICE_SUB_SYSTEM(p)	os_reg_pci_rx_u16(dev,0,0x2E)
#define	PCI_DEVICE_SUB_VENDOR(p)	os_reg_pci_rx_u16(dev,0,0x2C)
#define	PCI_DISABLE_DEVICE(p)
#define	PCI_ENABLE_DEVICE(p)		0
#define	PCI_SET_DMA_MASK(dev,mask)	pci_set_dma_mask((dev)->pci.pd,(dma_addr_t)(mask))
#define	PROC_GET_INFO(ptr,func)		(ptr)->get_info = (void*) (func)
#define	REGION_IO_CHECK(a,s)		check_region(a,s)
#define	REGION_IO_RELEASE(a,s)		release_region(a,s)
#define	REGION_IO_REQUEST(a,s,n)	(void*) (request_region(a,s,n), 1)
#define	REGION_MEM_CHECK(a,s)		0
#define	REGION_MEM_RELEASE(a,s)
#define	REGION_MEM_REQUEST(a,s,n)	(void*) 1
#define	REGION_TYPE_IO_BIT			1
#define	SET_CURRENT_STATE(s)		current->state = (s); mb()
#define	SET_MODULE_OWNER(p)
#define	VADDR_T						unsigned long
#define	WAIT_QUEUE_ENTRY_INIT(w,c)	(w)->task = (c)
#define	WAIT_QUEUE_ENTRY_T			struct wait_queue
#define	WAIT_QUEUE_HEAD_INIT(q)		init_waitqueue((q))
#define	WAIT_QUEUE_HEAD_T			struct wait_queue*

#define	IRQ_REQUEST(dv,fnc)			request_irq((dv)->pci.pd->irq,	\
												(fnc),				\
												SA_SHIRQ,			\
												DEV_NAME,			\
												(dv))

#ifdef GFP_DMA32
	#define	GFP_DMA_GSC				GFP_DMA32
#else
	#define	GFP_DMA_GSC				GFP_DMA
#endif



// data types *****************************************************************

typedef struct timeval	os_time_t;



// prototypes *****************************************************************

u32		os_bar_size(dev_data_t* dev, u8 bar);
void	os_irq_isr(int irq, void* dev_id, struct pt_regs* regs);
int		os_proc_get_info(char* page, char** start, off_t offset, int count, int dummy);



#endif
#endif

