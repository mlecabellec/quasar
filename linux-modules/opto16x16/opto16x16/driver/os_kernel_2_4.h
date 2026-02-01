// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_kernel_2_4.h $
// $Rev: 52470 $
// $Date: 2023-02-13 09:58:00 -0600 (Mon, 13 Feb 2023) $

// Device Driver: Linux: header file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#ifndef	__OS_KERNEL_2_4_H__
#define	__OS_KERNEL_2_4_H__

#include <linux/version.h>

#if	(LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)) && \
	(LINUX_VERSION_CODE <  KERNEL_VERSION(2,5,0))

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/page.h>
#include <asm/types.h>
#include <asm/uaccess.h>

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/iobuf.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/wrapper.h>



// macros *********************************************************************

#define	os_bar_pci_size(d,b)		pci_resource_len((d)->pci.pd,(b))

#define	PCI_CLEAR_MASTER(p)

#ifndef	min
	#define	min(a,b)				(((a) < (b)) ? (a) : (b))
#endif

#define	__GFP_NOWARN				0
#define	EVENT_RESUME_IRQ(q,c)		if (q) wake_up_interruptible(q)	// occurs inside ISR
#define	EVENT_WAIT_IRQ(q,c)			interruptible_sleep_on(q)
#define	EVENT_WAIT_IRQ_TO(q,c,t)	interruptible_sleep_on_timeout(q,t)
#define	FALLTHROUGH					// for the 5.9 kernel's "fallthrough" macro
#define	FASYNC_SET(_ptr,_func)		(_ptr)->fasync = (_func)
#define	GSC_HAVE_IOCTL_BKL			1	// Big Kernel Lock based ioctl()
#define	IOCTL_32BIT_SUPPORT			GSC_IOCTL_32BIT_NATIVE
#define	IOCTL_SET_BKL(_ptr,_func)	(_ptr)->ioctl = (_func)
#define	IOCTL_SET_COMPAT(p,f)
#define	IOCTL_SET_UNLOCKED(p,f)
#define	KILL_FASYNC(q,s,b)			kill_fasync(&(q),(s),(b))		// SIO4 specific
#define	MEM_ACCESS_OK(t,p,s)		access_ok((t),(p),(s))
#define	MEM_ALLOC_LIMIT				(2L * 1024L * 1024)
#undef	MOD_DEC_USE_COUNT
#define	MOD_DEC_USE_COUNT
#undef	MOD_INC_USE_COUNT
#define	MOD_INC_USE_COUNT
#define	OS_SUPPORTS_MSI				0
#define	PAGE_RESERVE(vpa)			mem_map_reserve(virt_to_page((vpa)))
#define	PAGE_UNRESERVE(vpa)			mem_map_unreserve(virt_to_page((vpa)))
#define	PCI_DEVICE_SUB_SYSTEM(p)	((p)->subsystem_device)
#define	PCI_DEVICE_SUB_VENDOR(p)	((p)->subsystem_vendor)
#define	PCI_DEVICE_LOOP(p)			pci_for_each_dev(p)
#define	PCI_DISABLE_DEVICE(p)		pci_disable_device((p))
#define	PCI_ENABLE_DEVICE(p)		pci_enable_device((p))
#define	PCI_SET_DMA_MASK(dev,mask)	pci_set_dma_mask((dev)->pci.pd,(dma_addr_t)(mask))
#define	PROC_GET_INFO(ptr,func)		(ptr)->get_info = (void*) (func)
#define	REGION_IO_CHECK(a,s)		check_region(a,s)
#define	REGION_IO_RELEASE(a,s)		release_region(a,s)
#define	REGION_IO_REQUEST(a,s,n)	request_region(a,s,n)
#define	REGION_MEM_CHECK(a,s)		check_mem_region(a,s)
#define	REGION_MEM_RELEASE(a,s)		release_mem_region(a,s)
#define	REGION_MEM_REQUEST(a,s,n)	request_mem_region(a,s,n)
#define	REGION_TYPE_IO_BIT			IORESOURCE_IO
#define	SET_CURRENT_STATE(s)		current->state = (s)
#define	VADDR_T						unsigned long
#define	WAIT_QUEUE_ENTRY_INIT(w,c)	init_waitqueue_entry((w),(c))
#define	WAIT_QUEUE_ENTRY_T			wait_queue_t
#define	WAIT_QUEUE_HEAD_INIT(q)		init_waitqueue_head((q))
#define	WAIT_QUEUE_HEAD_T			wait_queue_head_t

#ifdef GFP_DMA32
	#define	GFP_DMA_GSC				GFP_DMA32
#else
	#define	GFP_DMA_GSC				GFP_DMA
#endif

#define	IRQ_REQUEST(dv,fnc)			request_irq((dv)->pci.pd->irq,	\
												(fnc),				\
												SA_SHIRQ,			\
												DEV_NAME,			\
												(dv))

#define	 copy_from_user_ret(to, from, size, retval) {	\
	if(access_ok(VERIFY_READ, from, size)) {			\
		return(__copy_from_user(to, from, size));		\
	}													\
	else {												\
		return(retval);									\
	}													\
}

#define	 copy_to_user_ret(to, from, size, retval) {		\
	if(access_ok(VERIFY_WRITE, to, size)) {				\
		return(__copy_to_user(to, from, size));			\
	}													\
	else {												\
		return(retval);									\
	}													\
}

#define	put_user_ret(val, to, retval) {					\
	if(access_ok(VERIFY_WRITE, to, sizeof(*(to)))) {	\
		__put_user(val, to);							\
	}													\
	else {												\
		return(retval);									\
	}													\
 }

#define	get_user_ret(val, from, retval) {				\
	if(access_ok(VERIFY_READ, from, sizeof(*(from)))) {	\
		__get_user(val, from);							\
	}													\
	else {												\
		return(retval);									\
	}													\
}



// data types *****************************************************************

typedef struct timeval	os_time_t;



// prototypes *****************************************************************

void	os_irq_isr(int irq, void* dev_id, struct pt_regs* regs);
int		os_proc_get_info(char* page, char** start, off_t offset, int count);



#endif
#endif
