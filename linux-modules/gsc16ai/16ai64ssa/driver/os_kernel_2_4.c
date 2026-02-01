// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_kernel_2_4.c $
// $Rev: 53838 $
// $Date: 2023-11-15 13:51:51 -0600 (Wed, 15 Nov 2023) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include <linux/version.h>

#if	(LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)) && \
	(LINUX_VERSION_CODE <  KERNEL_VERSION(2,5,0))

#define	MODULE
#include "main.h"



// macros *********************************************************************

#ifndef	MODULE_LICENSE
	#define	MODULE_LICENSE(s)
#endif



// Tags ***********************************************************************

EXPORT_NO_SYMBOLS;
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
*		regs	Unused.
*
*	Returned:
*
*		None.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_IRQ
void os_irq_isr(int irq, void* dev_id, struct pt_regs* regs)
{
	gsc_irq_isr_common(dev_id, GSC_IRQ_ISR_FLAG_LOCK);
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
	{
		adrs[0]	= (unsigned long) dat;
	}
	else
	{
		// We don't report a failure here as the caller will adjust the
		// "order" and call us again. The caller is the one who will report
		// an error message when the time comes.
		adrs[0]	= 0;
	}

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
	MOD_DEC_USE_COUNT;
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
	MOD_INC_USE_COUNT;
	return(0);
}



/******************************************************************************
*
*	Function:	_proc_get_info
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
*	Returned:
*
*		int		The number of characters written.
*
******************************************************************************/

static int _proc_get_info(
	char*	page,
	char**	start,
	off_t	offset,
	int		count)
{
	int	eof;
	int	i;

	i	= os_proc_read(page, start, offset, count, &eof, NULL);
	return(i);
}



/******************************************************************************
*
*	Function:	os_proc_start_detail
*
*	Purpose:
*
*		Initialize use of the /proc file system.
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
		proc->get_info			= _proc_get_info;
	}
	else
	{
		ret	= -ENODEV;
		printf(	"%s: %d. %s: create_proc_entry() failure.\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__);
	}

	return(ret);
}



#endif
