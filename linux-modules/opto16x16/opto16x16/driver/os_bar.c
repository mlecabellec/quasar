// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_bar.c $
// $Rev: 51057 $
// $Date: 2022-05-03 16:24:02 -0500 (Tue, 03 May 2022) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



//*****************************************************************************
// io: <= 0 = do nothing, >0 = must be I/O mapped
// mem: <= 0 = do nothing, >0 = must be mem mapped
static int _bar_io_acquire(int io, int mem, os_bar_t* bar)
{
	int		ret;
	void*	vp;

	for (;;)	// A convenience loop.
	{
		if (mem > 0)
		{
			ret	= -EINVAL;
			printf(	"%s: %d. _bar_io_acquire: BAR%d is I/O mapped, but should be memory mapped.\n",
					DEV_NAME,
					__LINE__,
					bar->index);
			break;
		}

		if (io <= 0)
		{
			// Do nothing.
			ret	= 0;
			break;
		}

		if (bar->phys_adrs == 0)
		{
			ret	= -EINVAL;
			printf(	"%s: %d. _bar_io_acquire: BAR%d is not mapped into I/O space, but it should be.\n",
					DEV_NAME,
					__LINE__,
					bar->index);
			break;
		}

		if (bar->size == 0)
		{
			ret	= -EINVAL;
			printf(	"%s: %d. _bar_io_acquire: BAR%d is allotted no space.\n",
					DEV_NAME,
					__LINE__,
					bar->index);
			break;
		}

		ret	= REGION_IO_CHECK(bar->phys_adrs, bar->size);

		if (ret)
		{
			// This BAR region is already in use.
			ret	= -EALREADY;
			printf(	"%s: %d. _bar_io_acquire: BAR%d is already in use.\n",
					DEV_NAME,
					__LINE__,
					bar->index);
			break;
		}

		vp	= REGION_IO_REQUEST(bar->phys_adrs, bar->size, DEV_NAME);

		if (vp == NULL)
		{
			// This BAR region request failed.
			ret	= -EFAULT;
			printf(	"%s: %d. _bar_io_acquire: BAR%d region request failed.\n",
					DEV_NAME,
					__LINE__,
					bar->index);
			break;
		}

		// All went well.
		bar->requested	= 1;
		bar->vaddr		= (VADDR_T) bar->phys_adrs;
		break;
	}

	return(ret);
}




//*****************************************************************************
// io: <= 0 = do nothing, >0 = must be I/O mapped
// mem: <= 0 = do nothing, >0 = must be mem mapped
static int _bar_mem_acquire(int io, int mem, os_bar_t* bar)
{
	int		ret;
	void*	vp;

	for (;;)	// A convenience loop.
	{
		if (io > 0)
		{
			ret	= -EINVAL;
			printf(	"%s: %d. _bar_mem_acquire: BAR%d is I/O mapped, but should be memory mapped.\n",
					DEV_NAME,
					__LINE__,
					bar->index);
			break;
		}

		if (mem <= 0)
		{
			// Do nothing.
			ret	= 0;
			break;
		}

		if (bar->phys_adrs == 0)
		{
			ret	= -EINVAL;
			printf(	"%s: %d. _bar_mem_acquire: BAR%d is not mapped into memory space, but it should be.\n",
					DEV_NAME,
					__LINE__,
					bar->index);
			break;
		}

		if (bar->size == 0)
		{
			ret	= -EINVAL;
			printf(	"%s: %d. _bar_mem_acquire: BAR%d is allotted no space.\n",
					DEV_NAME,
					__LINE__,
					bar->index);
			break;
		}

		ret	= REGION_MEM_CHECK(bar->phys_adrs, bar->size);

		if (ret)
		{
			// This BAR region is already in use.
			ret	= -EALREADY;
			printf(	"%s: %d. _bar_mem_acquire: BAR%d is already in use.\n",
					DEV_NAME,
					__LINE__,
					bar->index);
			break;
		}

		vp	= (void*) REGION_MEM_REQUEST(bar->phys_adrs, bar->size, DEV_NAME);

		if (vp == NULL)
		{
			// This BAR region request failed.
			ret	= -EFAULT;
			printf(	"%s: %d. _bar_mem_acquire: BAR%d region request failed.\n",
					DEV_NAME,
					__LINE__,
					bar->index);
			break;
		}

		// All went well.
		bar->requested	= 1;
		bar->vaddr		= (VADDR_T) ioremap(bar->phys_adrs, bar->size);
		break;
	}

	return(ret);
}



//*****************************************************************************
// io: 0 = OK if not I/O mapped, >0 = must be I/O mapped, <0 = do not map
// mem: 0 = OK if not mem mapped, >0 = must be mem mapped, <0 = do not map
int os_bar_create(dev_data_t* dev, int index, int io, int mem, os_bar_t* bar)
{
	int	ret;

	memset(bar, 0,sizeof(os_bar_t));
	bar->dev		= dev;
	bar->index		= index;
	bar->offset		= 0x10 + 4 * index;
	bar->size		= (u32) os_bar_pci_size(dev, index);
	bar->reg		= os_reg_pci_rx_u32(dev, 1, bar->offset);

	if (bar->reg & D0)
	{
		bar->flags		= bar->reg & 0x3;
		bar->io_mapped	= 1;
		bar->phys_adrs	= bar->reg & 0xFFFFFFFC;
		ret				= _bar_io_acquire(io, mem, bar);
	}
	else
	{
		bar->flags		= bar->reg & 0xF;
		bar->io_mapped	= 0;
		bar->phys_adrs	= bar->reg & 0xFFFFFFF0;
		ret				= _bar_mem_acquire(io, mem, bar);
	}

	return(ret);
}



//*****************************************************************************
void os_bar_destroy(os_bar_t* bar)
{
	if (bar->requested == 0)
	{
		// There is nothing to release.
	}
	else if (bar->io_mapped)
	{
		REGION_IO_RELEASE(bar->phys_adrs, bar->size);
	}
	else
	{
		// Memory must be unmapped.

		if (bar->vaddr)
			iounmap((void*) bar->vaddr);

		REGION_MEM_RELEASE(bar->phys_adrs, bar->size);
	}

	memset(bar, 0,sizeof(os_bar_t));
}


