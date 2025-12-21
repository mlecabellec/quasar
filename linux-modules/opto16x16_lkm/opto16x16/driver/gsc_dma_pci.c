// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/pci_plx/gsc_dma_pci.c $
// $Rev: 50964 $
// $Date: 2022-04-25 08:31:03 -0500 (Mon, 25 Apr 2022) $

// OS & Device Independent: Device Driver: PLX: source file

#include "main.h"



// This source file is specific to the driver model used for DMA operations.
// This source file's content is specific to the PLX PCI9080, PCI9056 and
// PCI9656. The content is further specific to those drivers which access the
// control the DMA engines directly via virtual address (va or VADDR_T) reads
// and writes. Operations are limited to 32-bit DMA transfers.



// macros *********************************************************************

#ifndef	DEV_IO_AUTO_START
	#define	DEV_IO_AUTO_START(d,i)
#endif

#ifndef	GSC_DMA_CHANNEL_RESET_EXTERN
	#define	GSC_DMA_CHANNEL_RESET_EXTERN(dev, dma)
#endif



//*****************************************************************************
static int _dma_chan_abort(dev_data_t* dev, gsc_dma_ch_t* dma)
{
	int	aborted	= 0;
	u8 	csr;

	csr	= os_reg_mem_rx_u8(NULL, dma->vaddr.csr_8);

	if ((csr & GSC_DMA_CSR_DONE) == 0)
	{
		// Force DMA termination.
		// If the DMA has, by chance, already ended, then this abort
		// request will apply to the next DMA request on this channel.
		os_reg_mem_tx_u8(NULL, dma->vaddr.csr_8, GSC_DMA_CSR_ABORT);
		os_time_us_delay(10);		// Wait for completion.
		aborted	= 1;
	}

	return(aborted);
}



//*****************************************************************************
static int _dma_chan_init(dev_data_t* dev, int index)
{
	gsc_dma_ch_t*	dma	= &dev->dma.channel[index];
	int				ret	= 0;

	dma->in_use	= 0;
	_dma_chan_abort(dev, dma);

	os_reg_mem_tx_u8(NULL, dma->vaddr.csr_8, GSC_DMA_CSR_DISABLE);
	os_reg_mem_tx_u8(NULL, dma->vaddr.csr_8, GSC_DMA_CSR_CLEAR);

	// Allow external processing.
	GSC_DMA_CHANNEL_RESET_EXTERN(dev, dma);
	return(ret);
}



//*****************************************************************************
int gsc_dma_abort_pci(dev_data_t* dev, gsc_dma_ch_t* dma)
{
	int	aborted;

	if (dma->in_use)
		aborted	= _dma_chan_abort(dev, dma);
	else
		aborted	= 0;

	return(aborted);
}



//*****************************************************************************
gsc_dma_ch_t* gsc_dma_acquire_pci(dev_data_t* dev, u32 ability)
{
	gsc_dma_ch_t*	dma;

	for (;;)	// A convenience loop.
	{
		dma	= &dev->dma.channel[0];

		if ((dma->in_use == 0) && (dma->flags & ability))
		{
			dma->in_use	= 1;
			break;
		}

		dma	= &dev->dma.channel[1];

		if ((dma->in_use == 0) && (dma->flags & ability))
		{
			dma->in_use	= 1;
			break;
		}

		dma	= NULL;
		break;
	}

	return(dma);
}



//*****************************************************************************
int gsc_dma_close_pci(dev_data_t* dev)
{
	int	locked;
	int	ret	= 0;

	locked	= os_sem_lock(&dev->dma.sem);
	_dma_chan_init(dev, 0);
	_dma_chan_init(dev, 1);

	if (locked == 0)
		os_sem_unlock(&dev->dma.sem);

	return(ret);
}



//*****************************************************************************
int gsc_dma_create_pci(dev_data_t* dev, u32 ch0_flg, u32 ch1_flg)
{
	gsc_dma_ch_t*	dma;

	dev->vaddr.plx_dmaarb_32	= PLX_VADDR(dev, 0xAC);
	dev->vaddr.plx_dmathr_32	= PLX_VADDR(dev, 0xB0);

	dma					= &dev->dma.channel[0];
	dma->index			= 0;
	dma->flags			= ch0_flg;
	dma->intcsr_enable	= GSC_INTCSR_DMA_0_INT_ENABLE;
	dma->vaddr.mode_32	= PLX_VADDR(dev, 0x80);
	dma->vaddr.padr_32	= PLX_VADDR(dev, 0x84);
	dma->vaddr.ladr_32	= PLX_VADDR(dev, 0x88);
	dma->vaddr.siz_32	= PLX_VADDR(dev, 0x8C);
	dma->vaddr.dpr_32	= PLX_VADDR(dev, 0x90);
	dma->vaddr.csr_8	= PLX_VADDR(dev, 0xA8);

	dma					= &dev->dma.channel[1];
	dma->index			= 1;
	dma->flags			= ch1_flg;
	dma->intcsr_enable	= GSC_INTCSR_DMA_1_INT_ENABLE;
	dma->vaddr.mode_32	= PLX_VADDR(dev, 0x94);
	dma->vaddr.padr_32	= PLX_VADDR(dev, 0x98);
	dma->vaddr.ladr_32	= PLX_VADDR(dev, 0x9C);
	dma->vaddr.siz_32	= PLX_VADDR(dev, 0xA0);
	dma->vaddr.dpr_32	= PLX_VADDR(dev, 0xA4);
	dma->vaddr.csr_8	= PLX_VADDR(dev, 0xA9);
	return(0);
}



//*****************************************************************************
int gsc_dma_finish_pci(gsc_dma_setup_t* setup)
{
	u8				csr;
	gsc_dma_ch_t*	dma		= setup->dma;
	dev_data_t*		dev		= setup->dev;
	int				ret		= 0;

	// Check the results.
	csr	= os_reg_mem_rx_u8(dev, dma->vaddr.csr_8);

	// Disable the DONE interrupt.
	os_reg_mem_mx_u32(dev, dma->vaddr.mode_32, 0, GSC_DMA_MODE_INTERRUPT_WHEN_DONE);

	if ((csr & GSC_DMA_CSR_DONE) == 0)
	{
		//	If it isn't done then the operation times out.
		//	Unfortunately the PLX PCI9080 doesn't tell us
		//	how many bytes were transferred. So, instead
		//	of reporting the number of transferred bytes,
		//	or zero, we return an error status.

		// Force DMA termination.
		// If the DMA has, by chance, already ended, then this abort
		// request will apply to the next DMA request on this channel.
		os_reg_mem_tx_u8(dev, dma->vaddr.csr_8, GSC_DMA_CSR_ABORT);
		os_time_us_delay(10);		// Wait for completion.
		ret	= -ETIMEDOUT;
	}

	// Disable the DMA interrupt, just in case.
	os_reg_mem_mx_u32(dev, dev->vaddr.plx_intcsr_32, 0, dma->intcsr_enable);

	// Clear the DMA.
	os_reg_mem_tx_u8(dev, dma->vaddr.csr_8, GSC_DMA_CSR_CLEAR);

	return(ret);
}



//*****************************************************************************
int gsc_dma_open_pci(dev_data_t* dev)
{
	u16		did;
	VADDR_T	va;

	// Get the device id so we know which PLX chip we're dealing with.
	va	= PLX_VADDR(dev, 0x02);
	did	= os_reg_mem_rx_u16(NULL, va);

	switch (did)
	{
		default:
		case 0x9030:
		case 0x906E:

			break;

		case 0x9056:
		case 0x9080:
		case 0x9656:

			os_reg_mem_tx_u32(dev, dev->vaddr.plx_dmaarb_32, 0);
			os_reg_mem_tx_u32(dev, dev->vaddr.plx_dmathr_32, 0);

			// Change the DMA Read command from Memory Read Line to Memory Read
			// Multiple. This is the Control Register (CNTRL). We can't use the macro
			// name as we don't know which device headers are included.
			// Memory Read 0110b (6h) MR
			// Memory Read Line 1110b (Eh) MRL
			// Memory Read Multiple 1100b (Ch) MRM (multiple lines)
			va	= PLX_VADDR(dev, 0x6C);
			os_reg_mem_mx_u32(NULL, va, 0xC, 0xF);
			break;
	}

	_dma_chan_init(dev, 0);
	_dma_chan_init(dev, 1);
	return(0);
}



//*****************************************************************************
int gsc_dma_release_pci(dev_data_t* dev, gsc_dma_ch_t* dma)
{
	// No additional action required here.
	return(0);
}



//*****************************************************************************
int gsc_dma_setup_pci(gsc_dma_setup_t* setup)
{
	// Enable the DMA interrupt.
	os_reg_mem_mx_u32(	setup->dev,
						setup->dev->vaddr.plx_intcsr_32,
						setup->dma->intcsr_enable,
						setup->dma->intcsr_enable);

	// DMAMODEx
	os_reg_mem_tx_u32(	setup->dev,
						setup->dma->vaddr.mode_32,
						setup->mode);

	// DMAPADRx
	os_reg_mem_tx_u32(	setup->dev,
						setup->dma->vaddr.padr_32,
						(unsigned long) (u32) setup->mem->adrs);

	// DMALADRx
	os_reg_mem_tx_u32(	setup->dev,
						setup->dma->vaddr.ladr_32,
						setup->io->io_reg_offset);

	// DMASIZx
	os_reg_mem_tx_u32(	setup->dev,
						setup->dma->vaddr.siz_32,
						(u32) setup->bytes);

	// DMADPRx
	os_reg_mem_tx_u32(	setup->dev,
						setup->dma->vaddr.dpr_32,
						setup->dpr);

	return(0);
}



//*****************************************************************************
int gsc_dma_start_pci(GSC_ALT_STRUCT_T* alt, void* arg)
{
	gsc_dma_ch_t*		dma;
	gsc_dma_setup_t*	setup;
	unsigned long		ul;

	setup	= arg;
	dma		= setup->io->dma_channel;

	// Start the DMA transfer.
	os_reg_mem_tx_u8(NULL, dma->vaddr.csr_8, 0);		// Disabe
	ul	= GSC_DMA_CSR_ENABLE;
	os_reg_mem_tx_u8(NULL, dma->vaddr.csr_8, (u8) ul);	// Enable
	ul	= GSC_DMA_CSR_ENABLE | GSC_DMA_CSR_START;
	os_reg_mem_tx_u8(NULL, dma->vaddr.csr_8, (u8) ul);	// Start

	// Initiate any Auto Start activity.
	DEV_IO_AUTO_START(setup->alt, setup->io);

	return(0);
}


