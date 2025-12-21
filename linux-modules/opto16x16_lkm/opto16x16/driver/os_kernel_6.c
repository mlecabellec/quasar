// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_kernel_6.c $
// $Rev: 52658 $
// $Date: 2023-03-21 10:09:29 -0500 (Tue, 21 Mar 2023) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include <linux/version.h>

#if	(LINUX_VERSION_CODE >= KERNEL_VERSION(6,0,0)) && \
	(LINUX_VERSION_CODE <  KERNEL_VERSION(7,0,0))

#include <linux/module.h>
#include <linux/seq_file.h>

#include "main.h"



// Tags ***********************************************************************

MODULE_AUTHOR("General Standards Corporation, www.generalstandards.com");
MODULE_DESCRIPTION(DEV_MODEL " driver " GSC_DRIVER_VERSION);
MODULE_LICENSE("GPL");



// prototypes *****************************************************************

static int _proc_open(struct inode* inode, struct file* file);



// variables ******************************************************************

static const struct proc_ops	_proc_ops	=
{
	.proc_open	= _proc_open,
	.proc_read	= seq_read
};



//*****************************************************************************
void do_gettimeofday(os_time_t* tt)
{
	struct	timespec64	ts;

	ktime_get_real_ts64(&ts);
	tt->tv_sec	= ts.tv_sec;
	tt->tv_usec	= ts.tv_nsec / 1000;
}



/******************************************************************************
*
*	Function:	os_irq_isr
*
*	Purpose:
*
*		Service an interrupt.
*
*	Arguments:
*
*		irq		The interrupt number.
*
*		dev_id	The private data we've associated with the IRQ.
*
*		regs	Unused. Not present beginning at 2.6.19.
*
*	Returned:
*
*		The status as to whether we handled the interrupt or not.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_IRQ
irqreturn_t os_irq_isr(int irq, void* dev_id)
{
	int			is_ours;
	irqreturn_t	ret;

	is_ours	= gsc_irq_isr_common(dev_id, GSC_IRQ_ISR_FLAG_LOCK);
	ret		= is_ours ? IRQ_HANDLED : IRQ_NONE;
	return(ret);
}
#endif



/******************************************************************************
*
*	Function:	os_mem_dma_alloc_kernel
*
*	Purpose:
*
*		Kernel specific DMA memory allocation service.
*
*	Arguments:
*
*		dev		The structure for the device being accessed.
*
*		order	The power of two order for the allocation size.
*
*		adrs	The DMA engine address to use is returned here.
*
*	Returned:
*
*		None.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ) || defined(DEV_SUPPORTS_WRITE)
void* os_mem_dma_alloc_kernel(dev_data_t* dev, u8 order, unsigned long* adrs)
{
	dma_addr_t	dat;
	void*		ptr;

	// pci_alloc_consistent() last appeared in 5.17.18
//	ptr	= (void*) pci_alloc_consistent(dev->pci.pd, PAGE_SIZE << order, &dat);
	ptr	= (void*) dma_alloc_coherent(&dev->pci.pd->dev, PAGE_SIZE << order, &dat, GFP_ATOMIC);

	if (ptr)
		adrs[0]	= (unsigned long) dat;
	else
		adrs[0]	= 0;

	return(ptr);
}
#endif



/******************************************************************************
*
*	Function:	os_mem_dma_free_kernel
*
*	Purpose:
*
*		Kernel specific DMA memory free service.
*
*	Arguments:
*
*		mem		The memory to be freed.
*
*	Returned:
*
*		None.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ) || defined(DEV_SUPPORTS_WRITE)
void os_mem_dma_free_kernel(os_mem_t* mem)
{
	dma_free_coherent(&mem->dev->pci.pd->dev, PAGE_SIZE << mem->order, mem->ptr, mem->adrs);
}
#endif



/******************************************************************************
*
*	Function:	os_module_count_dec
*
*	Purpose:
*
*		Decrement the module usage count, as appropriate for the kernel.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void os_module_count_dec(void)
{
	module_put(THIS_MODULE);
}



/******************************************************************************
*
*	Function:	os_module_count_inc
*
*	Purpose:
*
*		Increment the module usage count, as appropriate for the kernel.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		0		All went well.
*		< 0		An appropriate error status.
*
******************************************************************************/

int os_module_count_inc(void)
{
	int	ret;

	ret	= try_module_get(THIS_MODULE);
	ret	= ret ? 0 : -ENODEV;
	return(ret);
}



//*****************************************************************************
static int _proc_show(struct seq_file* sf, void* vp)
{
	int		bytes	= 4096;
	int		eof		= 0;
	char*	ptr		= NULL;
	int		ret;

	for (;;)	// A convenience loop.
	{
		ptr	= kmalloc(bytes, GFP_KERNEL);

		if (ptr == NULL)
		{
			ret	= -ENOMEM;
			break;
		}

		ret	= os_proc_read(ptr, &ptr, 0, bytes, &eof, NULL);

		if (ret)
		{
			seq_printf(sf, "%s", ptr);
			kfree(ptr);
			ret	= 0;
			break;
		}

		ret	= -EINVAL;
		break;
	}

	return(ret);
}



//*****************************************************************************
static int _proc_open(struct inode* inode, struct file* file)
{
	int	ret;

	ret	= single_open(file, _proc_show, NULL);
	return(ret);
}



//*****************************************************************************
int os_proc_start_detail(void)
{
	struct proc_dir_entry*	proc;
	int						ret;

	proc	= proc_create(DEV_NAME, 0, NULL, &_proc_ops);

	if (proc)
	{
		ret						= 0;
		gsc_global.proc_enabled	= 1;
	}
	else
	{
		ret	= -ENODEV;
		printf(	"%s: os_proc_start_detail:"
				" proc_create() failure.\n",
				DEV_NAME);
	}

	return(ret);
}



#endif
