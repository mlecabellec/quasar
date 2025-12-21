// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_mem.c $
// $Rev: 52470 $
// $Date: 2023-02-13 09:58:00 -0600 (Mon, 13 Feb 2023) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



/******************************************************************************
*
*	Function:	_order_to_size
*
*	Purpose:
*
*		Convert an order value to its corresponding size in page sized units.
*
*	Arguments:
*
*		order	The page size order to convert.
*
*	Returned:
*
*		>= 0	The order corresponding to the given size.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ) || defined(DEV_SUPPORTS_WRITE)
static u32 _order_to_size(int order)
{
	u32	size	= PAGE_SIZE;

	// Limit sizes to 1G.

	for (; order > 0; order--)
	{
		if (size >= 0x40000000)
			break;

		size	*= 2;
	}

	return(size);
}
#endif



/******************************************************************************
*
*	Function:	_pages_free
*
*	Purpose:
*
*		Free memory allocated via _pages_alloc().
*
*	Arguments:
*
*		io		Parameters for the allocation to be freed.
*
*	Returned:
*
*		None.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ) || defined(DEV_SUPPORTS_WRITE)
static void _pages_free(os_mem_t* mem)
{
	s32				bytes;
	unsigned long	ul;

	if (mem->ptr)
	{
		bytes	= mem->bytes;
		ul		= (unsigned long) mem->ptr;

		for (; bytes >= PAGE_SIZE; bytes -= PAGE_SIZE, ul += PAGE_SIZE)
			PAGE_UNRESERVE(ul);

		// Free the memory.
		os_mem_dma_free_kernel(mem);
		mem->adrs		= 0;
		mem->ptr		= NULL;
		mem->bytes		= 0;
		mem->order		= 0;
	}
}
#endif



/******************************************************************************
*
*	Function:	_size_to_order
*
*	Purpose:
*
*		Convert a size value to a corresponding page size order value.
*
*	Arguments:
*
*		size	The desired size in bytes.
*
*	Returned:
*
*		>= 0	The order corresponding to the given size.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ) || defined(DEV_SUPPORTS_WRITE)
static int _size_to_order(u32 size)
{
	u32	bytes	= PAGE_SIZE;
	int	order	= 0;

	// Limit sizes to 1G.

	for (; bytes < 0x40000000;)
	{
		if (bytes >= size)
			break;

		order++;
		bytes	*= 2;
	}

	return(order);
}
#endif



//*****************************************************************************
int os_mem_copy_from_user(void* dst, const void* src, long size)
{
	int				ret;
	unsigned long	ul;

	for (;;)	// A convenience loop.
	{
		ul	= MEM_ACCESS_OK(VERIFY_READ, (void*) src, size);

		if (ul == 0)
		{
			// We can't read from the user's memory.
			ret	= -EFAULT;
			break;
		}

		ul	= copy_from_user(dst, (void*) src, size);

		if (ul)
		{
			// We couldn't read from the user's memory.
			ret	= -EFAULT;
			break;
		}

		ret	= 0;
		break;
	}

	return(ret);
}



//*****************************************************************************
int os_mem_copy_to_user(void* dst, const void* src, long size)
{
	int				ret;
	unsigned long	ul;

	for (;;)	// A convenience loop.
	{
		ul	= MEM_ACCESS_OK(VERIFY_WRITE, dst, size);

		if (ul == 0)
		{
			// We can't write to the user's memory.
			ret	= -EFAULT;
			break;
		}

		ul	= copy_to_user(dst, (void*) src, size);

		if (ul)
		{
			// We couldn't write to the user's memory.
			ret	= -EFAULT;
			break;
		}

		ret	= 0;
		break;
	}

	return(ret);
}



//*****************************************************************************
void* os_mem_data_alloc(size_t size)
{
	void*	ptr;

	ptr	= kmalloc(size, GFP_KERNEL);

	if (ptr)
		memset(ptr, 0, size);
	else
		printf("%s: os_mem_data_alloc(%ld) failed.\n", DEV_NAME, (long) size);

	return(ptr);
}



//*****************************************************************************
void os_mem_data_free(void* ptr)
{
	if (ptr)
		kfree(ptr);
}



//*****************************************************************************
#if defined(DEV_SUPPORTS_READ) || defined(DEV_SUPPORTS_WRITE)
void* os_mem_dma_alloc(GSC_ALT_STRUCT_T* alt, size_t* bytes, os_mem_t* mem)
{
	dev_data_t*		dev		= GSC_ALT_DEV_GET(alt);
	u8				order;
	void*			ptr		= NULL;
	int				ret;
	unsigned long	tmp;
	unsigned long	ul;

	if (bytes[0] > MEM_ALLOC_LIMIT)
		bytes[0]	= MEM_ALLOC_LIMIT;

	order		= _size_to_order(bytes[0]);
	bytes[0]	= _order_to_size(order);
	ret			= PCI_SET_DMA_MASK(dev, 0xFFFFFFFF);

	if (ret)
		printf("%s: os_mem_dma_alloc(): mask set failure: ret %d\n", DEV_NAME, ret);

	for (; ret == 0;)
	{
		ptr	= os_mem_dma_alloc_kernel(dev, order, &mem->adrs);

		if (ptr)
			break;

		if (order == 0)
			break;

		order--;
		bytes[0]	/= 2;
	}

	if (ptr == NULL)
	{
		mem->ptr	= NULL;
		mem->bytes	= bytes[0];
		mem->order	= 0;
		mem->dev	= NULL;
		printf("%s: os_mem_dma_alloc(%ld) failed.\n", DEV_NAME, (long) bytes[0]);
	}
	else
	{
		memset(ptr, 0, bytes[0]);
		mem->ptr	= ptr;
		mem->bytes	= bytes[0];
		mem->order	= order;
		mem->dev	= dev;

		// Mark the pages as reserved so the MM will leave'm alone.
		ul	= (unsigned long) mem->ptr;
		tmp	= bytes[0];

		for (; tmp >= PAGE_SIZE; tmp -= PAGE_SIZE, ul += PAGE_SIZE)
			PAGE_RESERVE(ul);
	}

	return(ptr);
}
#endif



//*****************************************************************************
#if defined(DEV_SUPPORTS_READ) || defined(DEV_SUPPORTS_WRITE)
void os_mem_dma_close(os_mem_t* mem)
{
	// Nothing required under Linux.
}
#endif



//*****************************************************************************
#if defined(DEV_SUPPORTS_READ) || defined(DEV_SUPPORTS_WRITE)
void os_mem_dma_free(os_mem_t* mem)
{
	if (mem)
		_pages_free(mem);
}
#endif



//*****************************************************************************
#if defined(DEV_SUPPORTS_READ) || defined(DEV_SUPPORTS_WRITE)
void os_mem_dma_open(os_mem_t* mem)
{
	// Nothing required under Linux.
}
#endif


