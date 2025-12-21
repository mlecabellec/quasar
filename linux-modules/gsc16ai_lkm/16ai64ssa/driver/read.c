// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/read.c $
// $Rev: 51346 $
// $Date: 2022-07-11 17:38:48 -0500 (Mon, 11 Jul 2022) $

// 16AI64SSA: Device Driver: source file

#include "main.h"



//*****************************************************************************
static void _dev_io_sw_init(dev_data_t* dev, dev_io_t* io)
{
	io->bytes_per_sample	= 4;
	io->io_mode				= AI64SSA_IO_MODE_DEFAULT;
	io->overflow_check		= AI64SSA_IO_OVERFLOW_DEFAULT;
	io->pio_threshold		= 32;
	io->timeout_s			= AI64SSA_IO_TIMEOUT_DEFAULT;
	io->underflow_check		= AI64SSA_IO_UNDERFLOW_DEFAULT;
}



//*****************************************************************************
static void _dev_io_close(dev_data_t* dev, dev_io_t* io)
{
	io->dma_channel	= NULL;
}



//*****************************************************************************
static void _dev_io_open(dev_data_t* dev, dev_io_t* io)
{
	_dev_io_sw_init(dev, io);
}



//*****************************************************************************
static int _dev_io_startup(dev_data_t* dev, dev_io_t* io)
{
	u32		bctlr;
	long	ret		= 0;

	if ((io->overflow_check) || (io->underflow_check))
	{
		bctlr	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_bctlr_32);

		if ((io->overflow_check) && (bctlr & D17))
			ret	= -EIO;

		if ((io->underflow_check) && (bctlr & D16))
			ret	= -EIO;
	}

	return(ret);
}



//*****************************************************************************
static long _dev_pio_available(dev_data_t* dev, dev_io_t* io, size_t count)
{
	u32	bufsr;

	bufsr	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_bufsr_32);

	if (dev->cache.pci9080)
		count	= (bufsr & 0x3FFFF) * 4;
	else
		count	= (bufsr & 0x7FFFF) * 4;

	return(count);
}



//*****************************************************************************
static long _dev_pio_xfer(
	dev_data_t*		dev,
	dev_io_t*		io,
	const os_mem_t*	mem,
	size_t			count,
	os_time_tick_t	st_end)
{
	long	qty;

	qty	= gsc_read_pio_work_32_bit(dev, io, mem, count, st_end);
	return(qty);
}



//*****************************************************************************
static long _dev_bmdma_available(dev_data_t* dev, dev_io_t* io, size_t count)
{
	u32	avail;
	u32	bufsr;
	u32	ibcr;
	u32	thresh;

	for (;;)
	{
		// Read from the current content, if there is enough data.
		bufsr	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_bufsr_32);

		if (dev->cache.pci9080)
			avail	= (bufsr & 0x3FFFF) * 4;
		else
			avail	= (bufsr & 0x7FFFF) * 4;

		if (count <= avail)
		{
			// Read from the available data.
			break;
		}

		// Read from the current content, if the threshold has been met.
		ibcr	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_ibcr_32);

		if (dev->cache.pci9080)
		{
			thresh	= (ibcr & 0xFFFF) * 4;

			if (ibcr & D19)
				thresh	*= 4;
		}
		else
		{
			thresh	= (ibcr & 0x3FFFF) * 4;
		}

		if (avail >= thresh)
		{
			// Read upto the threshold level.
			count	= thresh;
			break;
		}

		// Wait until additional data becomes available.
		count	= 0;
		break;
	}

	return(count);
}



//*****************************************************************************
static long _dev_bmdma_xfer(
	dev_data_t*		dev,
	dev_io_t*		io,
	const os_mem_t*	mem,
	size_t			count,
	os_time_tick_t	st_end)
{
	long			qty;
	long			samples	= count / 4;
	gsc_dma_setup_t	setup;

	if (samples < io->pio_threshold)
	{
		qty	= gsc_read_pio_work_32_bit(dev, io, mem, count, st_end);
	}
	else
	{
		memset(&setup, 0, sizeof(gsc_dma_setup_t));
		setup.alt		= dev;
		setup.dev		= dev;
		setup.io		= io;
		setup.mem		= mem;
		setup.st_end	= st_end;
		setup.bytes		= count;
		setup.ability	= GSC_DMA_CAP_BMDMA_READ;

		setup.mode		= GSC_DMA_MODE_BLOCK_DMA
						| GSC_DMA_MODE_SIZE_32_BITS
						| GSC_DMA_MODE_INPUT_ENABLE
						| GSC_DMA_MODE_BURSTING_LOCAL
						| GSC_DMA_MODE_INTERRUPT_WHEN_DONE
						| GSC_DMA_MODE_LOCAL_ADRESS_CONSTANT
						| GSC_DMA_MODE_PCI_INTERRUPT_ENABLE;

		setup.dpr		= GSC_DMA_DPR_BOARD_TO_HOST
						| GSC_DMA_DPR_END_OF_CHAIN
						| GSC_DMA_DPR_TERMINAL_COUNT_IRQ;

		qty	= gsc_dma_perform(&setup);
	}

	return(qty);
}



//*****************************************************************************
static long _dev_dmdma_available(dev_data_t* dev, dev_io_t* io, size_t count)
{
	if (dev->cache.pci9080)
		count	= _dev_bmdma_available(dev, io, count);

	return(count);
}



//*****************************************************************************
static long _dev_dmdma_xfer(
	dev_data_t*		dev,
	dev_io_t*		io,
	const os_mem_t*	mem,
	size_t			count,
	os_time_tick_t	st_end)
{
	u32				bctlr;
	long			qty;
	long			samples	= count / 4;
	gsc_dma_setup_t	setup;

	if (samples < io->pio_threshold)
	{
		qty	= gsc_read_pio_work_32_bit(dev, io, mem, count, st_end);
	}
	else
	{
		bctlr	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_bctlr_32);

		if (dev->cache.pci9080)
		{
			// Enable DMDMA.

			if (bctlr & D19)
				os_reg_mem_mx_u32(dev, dev->vaddr.gsc_bctlr_32, 0, D12 | D13 | D15 | D19);
		}
		else
		{
			// Enable autonomous DMDMA.

			if ((bctlr & D19) == 0)
				os_reg_mem_mx_u32(dev, dev->vaddr.gsc_bctlr_32, D12 | D13 | D15 | D19, D19);
		}

		memset(&setup, 0, sizeof(gsc_dma_setup_t));
		setup.alt		= dev;
		setup.dev		= dev;
		setup.io		= io;
		setup.mem		= mem;
		setup.st_end	= st_end;
		setup.bytes		= count;
		setup.ability	= GSC_DMA_CAP_DMDMA_READ;

		setup.mode		= GSC_DMA_MODE_DM_DMA
						| GSC_DMA_MODE_SIZE_32_BITS
						| GSC_DMA_MODE_INPUT_ENABLE
						| GSC_DMA_MODE_BURSTING_LOCAL
						| GSC_DMA_MODE_INTERRUPT_WHEN_DONE
						| GSC_DMA_MODE_LOCAL_ADRESS_CONSTANT
						| GSC_DMA_MODE_PCI_INTERRUPT_ENABLE;

		setup.dpr		= GSC_DMA_DPR_BOARD_TO_HOST
						| GSC_DMA_DPR_END_OF_CHAIN
						| GSC_DMA_DPR_TERMINAL_COUNT_IRQ;

		qty	= gsc_dma_perform(&setup);
	}

	return(qty);
}



/******************************************************************************
*
*	Function:	dev_read_create
*
*	Purpose:
*
*		Perform a one-tine setup of the Read Analog Input streaming I/O structure.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*		io		The I/O structure for this stream.
*
*	Returned:
*
*		None.
*
******************************************************************************/

int dev_read_create(dev_data_t* dev, dev_io_t* io)
{
	int	ret;

	io->bytes_per_sample	= 4;
	io->io_reg_offset		= GSC_REG_OFFSET(AI64SSA_GSC_IDBR);
	io->io_reg_vaddr		= dev->vaddr.gsc_idbr_32;

	io->dev_io_sw_init		= _dev_io_sw_init;
	io->dev_io_close		= _dev_io_close;
	io->dev_io_open			= _dev_io_open;
	io->dev_io_startup		= _dev_io_startup;
	io->dev_pio_available	= _dev_pio_available;
	io->dev_bmdma_available	= _dev_bmdma_available;
	io->dev_dmdma_available	= _dev_dmdma_available;
	io->dev_pio_xfer		= _dev_pio_xfer;
	io->dev_bmdma_xfer		= _dev_bmdma_xfer;
	io->dev_dmdma_xfer		= _dev_dmdma_xfer;

	io->wait.abort			= AI64SSA_WAIT_IO_RX_ABORT;
	io->wait.done			= AI64SSA_WAIT_IO_RX_DONE;
	io->wait.error			= AI64SSA_WAIT_IO_RX_ERROR;
	io->wait.timeout		= AI64SSA_WAIT_IO_RX_TIMEOUT;

	ret	= gsc_io_create(dev, io, 4L * 1024L * 1024L);

	return(ret);
}


