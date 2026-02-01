// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_kernel_3.c $
// $Rev: 49674 $
// $Date: 2021-09-30 22:29:40 -0500 (Thu, 30 Sep 2021) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include <linux/version.h>

#if	(LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)) && \
	(LINUX_VERSION_CODE <  KERNEL_VERSION(4,0,0))

#include <linux/module.h>
#include <linux/seq_file.h>

#include "main.h"



// Tags ***********************************************************************

MODULE_AUTHOR("General Standards Corporation, www.generalstandards.com");
MODULE_DESCRIPTION(DEV_MODEL " driver " GSC_DRIVER_VERSION);
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE(DEV_MODEL);



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

	ptr	= (void*) pci_alloc_consistent(dev->pci.pd, PAGE_SIZE << order, &dat);

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
	pci_free_consistent(mem->dev->pci.pd, PAGE_SIZE << mem->order, mem->ptr, mem->adrs);
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



#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,9,0))
//*****************************************************************************
int os_proc_start_detail(void)
{
	struct proc_dir_entry*	proc;
	int						ret;

	proc	= create_proc_entry(DEV_NAME, S_IRUGO, NULL);

	if (proc)
	{
		ret						= 0;
		gsc_global.proc_enabled	= 1;
		proc->read_proc			= os_proc_read;
	}
	else
	{
		ret	= -ENODEV;
		printf(	"%s: os_proc_start_detail:"
				" create_proc_entry() failure.\n",
				DEV_NAME);
	}

	return(ret);
}



#else
//*****************************************************************************
static int _proc_show(struct seq_file* sf, void* vp)
{
	int		bytes	= 4096;
	int		eof		= 0;
	char*	ptr		= NULL;
	int		ret		= -ENOMEM;

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
			ptr	= NULL;
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
	static const struct file_operations	fops	=
	{
		.open	= _proc_open,
		.read	= seq_read
	};

	struct proc_dir_entry*	proc;
	int						ret;

	proc	= proc_create(DEV_NAME, 0, NULL, &fops);

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



#endif
