// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/device.c $
// $Rev: 53568 $
// $Date: 2023-08-07 16:29:27 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Device Driver: source file

#include "main.h"



// variables ******************************************************************

const gsc_dev_id_t	dev_id_list[]	=
{
	// model		Vendor	Device	SubVen	SubDev	type

	{ "16AI64SSA",	0x10B5, 0x9056, 0x10B5, 0x3101,	GSC_DEV_TYPE_16AI64SSA	},
	{ "16AI64SSA",	0x10B5, 0x9080, 0x10B5, 0x2868,	GSC_DEV_TYPE_16AI64SSA	},
	{ NULL }
};



//*****************************************************************************
static void _chan_range_compute(dev_data_t* dev)
{
	u32	rev	= GSC_FIELD_DECODE(dev->cache.gsc_bcfgr_32, 11, 0);

	if (dev->cache.pci9080)
		dev->cache.channel_range	= ((rev >= 0x200) && (rev <= 0x4ff)) ? 1 : 0;
	else
		dev->cache.channel_range	= (rev >= 0x100) ? 1 : 0;
}



//*****************************************************************************
static void _channels_compute(dev_data_t* dev)
{
	if (dev->cache.gsc_bcfgr_32 & 0x10000)
		dev->cache.channel_qty	= 32;
	else
		dev->cache.channel_qty	= 64;

	dev->cache.channels_max	= 64;
}



//*****************************************************************************
static void _irq1_buf_error_compute(dev_data_t* dev)
{
	dev->cache.irq1_buf_error	= dev->cache.pci9080 ? 0 : 1;
}



//*****************************************************************************
static void _master_clock_compute(dev_data_t* dev)
{
	if (dev->cache.pci9080)
	{
		dev->cache.master_clock	= 30000000L;
	}
	else
	{
		switch (dev->cache.gsc_bcfgr_32 & 0x60000)
		{
			default:
			case 0x00000:	dev->cache.master_clock	= 50000000L;	break;
			case 0x20000:	dev->cache.master_clock	= 45000000L;	break;
			case 0x40000:	dev->cache.master_clock	= 49152000L;	break;
			case 0x60000:	dev->cache.master_clock	= 51840000L;	break;
		}
	}
}



//*****************************************************************************
static void _low_latency_compute(dev_data_t* dev)
{
	u32	llcr;

	if (dev->cache.pci9080)
		dev->cache.low_latency	= 0;
	else
		dev->cache.low_latency	= (dev->cache.gsc_bcfgr_32 & D15) ? 1 : 0;

	initialize_ioctl(dev, NULL);
	llcr	= os_reg_mem_rx_u32(NULL, dev->vaddr.gsc_llcr_32);

	if (dev->cache.low_latency == 0)
		dev->cache.reg_llcr	= 0;
	else if (dev->cache.channel_qty == 64)
		dev->cache.reg_llcr	= ((llcr & 0xFFF) == (0x3f << 6)) ? 1 : 0;
	else
		dev->cache.reg_llcr	= ((llcr & 0xFFF) == (0x1f << 6)) ? 1 : 0;
}



//*****************************************************************************
static void _memory_compute(dev_data_t* dev)
{
	if (dev->cache.pci9080 == 0)
		dev->cache.fifo_size	= _256K;
	else if (dev->cache.gsc_bcfgr_32 & D18)
		dev->cache.fifo_size	= _256K;
	else
		dev->cache.fifo_size	= _64K;
}



//*****************************************************************************
static void _reg_acar_compute(dev_data_t* dev)
{
	u32	rev	= GSC_FIELD_DECODE(dev->cache.gsc_bcfgr_32, 11, 0);

	if (dev->cache.pci9080)
		dev->cache.reg_acar	= ((rev >= 0x200) && (rev <= 0x4ff)) ? 1 : 0;
	else
		dev->cache.reg_acar	= (rev >= 0x100) ? 1 : 0;
}



//****************************************************************************(
static int _type_compute(dev_data_t* dev)
{
	u32		_0x40;
	VADDR_T	_0x40_va;
	u32		_0x80;
	VADDR_T	_0x80_va;
	u32		bcfgr;
	VADDR_T	bcfgr_va;
	u32		bctlr;
	VADDR_T	bctlr_va;
	u32		fw;
	VADDR_T	icr_va;
	u32		intcsr;
	VADDR_T	intcsr_va;
	u32		ragr;
	VADDR_T	ragr_va;
	int		ret;

	_0x40_va	= GSC_VADDR(dev, 0x40);
	_0x80_va	= GSC_VADDR(dev, 0x80);
	bcfgr_va	= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_BCFGR));
	bctlr_va	= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_BCTLR));

	_0x40		= os_reg_mem_rx_u32(NULL, _0x40_va);
	_0x80		= os_reg_mem_rx_u32(NULL, _0x80_va);
	bcfgr		= os_reg_mem_rx_u32(NULL, bcfgr_va);
	bctlr		= os_reg_mem_rx_u32(NULL, bctlr_va);

	if ((bctlr == _0x40) && (bctlr == _0x80) && ((bcfgr & D15) == 0))
	{
		// 16AI64SSA: @0x40 == BCR, @0x80 == BCR, BCFGR.D15 == 0, Firmware == 0x0100-0x02xx
		// 16AI64SSC: @0x40 == BCR, @0x80 == BCR, BCFGR.D15 == 0, Firmware != 0x0100-0x02xx
		// This IS one of our devices.
		ret	= 0;
		fw	= bcfgr & 0xFFFF;

		if ((fw < 0x0100) || (fw > 0x02FF))
		{
			dev->model		= "16AI64SSC";
			dev->board_type	= GSC_DEV_TYPE_16AI64SSC;
		}
	}
	else if ((bctlr == _0x80) && (bcfgr & D15))
	{
		// This is either a 16AI64SSC or a 16AI32SSA.
		// 16AI64SSC: @0x40 == BCR, @0x80 == BCR, BCFGR.D15 == 1, RAGR == 0x00010FA0
		// 16AI64SSC: @0x40 != BCR, @0x80 == BCR, BCFGR.D15 == 1, RAGR == 0x00010FA0
		// Reset the device so we can examine the Rate-A Generator Register.
		// Disable interrupts as the 16AI32SSA causes an Init Done IRQ.
		icr_va		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_ICR));
		intcsr_va	= PLX_VADDR(dev, 0x68);
		ragr_va		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_RAGR));

		intcsr		= os_reg_mem_rx_u32(NULL, intcsr_va);	// Save INTCSR.
		os_reg_mem_tx_u32(NULL, intcsr_va, 0);				// Clear INTCSR.
		os_reg_mem_mx_u32(NULL, bctlr_va, D15, D15);		// Initialize
		os_time_sleep_ms(10);								// Wait til done.
		os_reg_mem_tx_u32(NULL, icr_va, 0);					// Clear the IRQ.
		os_reg_mem_tx_u32(NULL, intcsr_va, intcsr);			// Restore INTCSR.

		// Now examine the Rate-A Generator Register.
		ragr	= os_reg_mem_rx_u32(NULL, ragr_va);

		if (ragr == 0x00010FA0)
		{
			// This IS one of our devices.
			ret				= 0;
			dev->model		= "16AI64SSC";
			dev->board_type	= GSC_DEV_TYPE_16AI64SSC;
		}
		else
		{
			// This is NOT one of our devices.
			ret	= -ENODEV;
#if DEV_PCI_ID_SHOW
			printk("This device is not supported by this driver.\n");
#endif
		}
	}
	else
	{
		ret	= -ENODEV;	// This is NOT one of our devices.
#if DEV_PCI_ID_SHOW
		printk("This device is not supported by this driver.\n");
#endif
	}

	return(ret);
}



//*****************************************************************************
static void _icr_anomaly_compute(dev_data_t* dev)
{
	u16	fw	= dev->cache.gsc_bcfgr_32 & 0xFFF;

	if (dev->pci.pd->device == 0x9080)
		dev->cache.icr_anomaly	= (fw >= 0x202) ? 0 : 1;
	else if (dev->pci.pd->device == 0x9056)
		dev->cache.icr_anomaly	= (fw >= 0x222) ? 0 : 1;
	else
		dev->cache.icr_anomaly	= 1;
}



/******************************************************************************
*
*	Function:	dev_device_create
*
*	Purpose:
*
*		Do everything needed to setup and use the given device.
*
*	Arguments:
*
*		dev		The structure to initialize.
*
*	Returned:
*
*		0		All is well.
*		< 0		An appropriate error status.
*
******************************************************************************/

int dev_device_create(dev_data_t* dev)
{
	static const gsc_bar_maps_t	bar_map	=
	{
		{
			// mem	io	rw
			{ 1,	0,	GSC_REG_TYPE_ACCESS_RO },	// BAR 0: PLX registers, memory mapped
			{ 0,	0,	GSC_REG_TYPE_ACCESS_RO },	// BAR 1: PLX registers, I/O mapped
			{ 1,	0,	GSC_REG_TYPE_ACCESS_RW },	// BAR 2: GSC registers, memory mapped
			{ 0,	0,	GSC_REG_TYPE_ACCESS_RO },	// BAR 3: unused
			{ 0,	0,	GSC_REG_TYPE_ACCESS_RO },	// BAR 4: unused
			{ 0,	0,	GSC_REG_TYPE_ACCESS_RO }	// BAR 5: unused
		}
	};

	u32	dma;
	int	ret;

	for (;;)	// A convenience loop.
	{
		// Verify some macro contents.
		ret	= gsc_macro_test_base_name(AI64SSA_BASE_NAME);
		if (ret)	break;

		ret	= gsc_macro_test_model();
		if (ret)	break;

		// PCI setup.
		ret	= os_pci_dev_enable(&dev->pci);
		if (ret)	break;

		ret	= os_pci_master_set(&dev->pci);
		if (ret)	break;

		// Control ISR access to the device and data structure.
		ret	= os_spinlock_create(&dev->spinlock);
		if (ret)	break;

		// Control access to the device and data structure.
		ret	= os_sem_create(&dev->sem);
		if (ret)	break;

		// Access the BAR regions.
		ret	= gsc_bar_create(dev, &dev->bar, &bar_map);
		if (ret)	break;

		// Verify that this is a supported device.
		ret	= _type_compute(dev);
		if (ret)	break;

		// Firmware access.
		dev->vaddr.gsc_acar_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_ACAR));
		dev->vaddr.gsc_asiocr_32	= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_ASIOCR));
		dev->vaddr.gsc_bcfgr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_BCFGR));
		dev->vaddr.gsc_bctlr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_BCTLR));
		dev->vaddr.gsc_bufsr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_BUFSR));
		dev->vaddr.gsc_bursr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_BURSR));
		dev->vaddr.gsc_ibcr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_IBCR));
		dev->vaddr.gsc_icr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_ICR));
		dev->vaddr.gsc_idbr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_IDBR));
		dev->vaddr.gsc_llcr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_LLCR));
		dev->vaddr.gsc_llhr00_16	= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_LLHR00));
		dev->vaddr.gsc_ragr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_RAGR));
		dev->vaddr.gsc_rbgr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_RBGR));
		dev->vaddr.gsc_smlwr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_SMLWR));
		dev->vaddr.gsc_smuwr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_SMUWR));
		dev->vaddr.gsc_sscr_32		= GSC_VADDR(dev, GSC_REG_OFFSET(AI64SSA_GSC_SSCR));

		// Data cache initialization.
		dev->cache.gsc_bcfgr_32		= os_reg_mem_rx_u32(NULL, dev->vaddr.gsc_bcfgr_32);

		dev->cache.autocal_ms		= 500;
		dev->cache.data_packing		= (dev->pci.pd->device == 0x9080) ? 0 : 1;
		dev->cache.fsamp_max		= 200000L;
		dev->cache.fsamp_min		= 1L;
		dev->cache.initialize_ms	= 3;
		dev->cache.pci9080			= (dev->pci.pd->device == 0x9080) ? 1 : 0;
		dev->cache.rate_gen_qty		= 2;

		_channels_compute(dev);
		_icr_anomaly_compute(dev);
		_master_clock_compute(dev);
		_memory_compute(dev);
		_low_latency_compute(dev);	// after channels
		_chan_range_compute(dev);
		_reg_acar_compute(dev);
		_irq1_buf_error_compute(dev);

		dev->cache.nrate_max		= 0xFFFF;
		dev->cache.nrate_min		= dev->cache.master_clock / dev->cache.fsamp_max;

		// Initialize additional resources.
		ret	= dev_irq_create(dev);
		if (ret)	break;

		ret	= dev_io_create(dev);
		if (ret)	break;

		dma	= GSC_DMA_SEL_STATIC
			| GSC_DMA_CAP_BMDMA_READ
			| GSC_DMA_CAP_DMDMA_READ;
		ret	= gsc_dma_create(dev, dma, dma);

		if (ret == 0)
			os_metrics(dev, dev->vaddr.gsc_bctlr_32);

		break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	dev_device_destroy
*
*	Purpose:
*
*		Do everything needed to release the referenced device.
*
*	Arguments:
*
*		dev		The partial data for the device of interest.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void dev_device_destroy(dev_data_t* dev)
{
	if (dev)
	{
		gsc_dma_destroy(dev);
		dev_io_destroy(dev);
		dev_irq_destroy(dev);
		gsc_bar_destroy(&dev->bar);
		os_sem_destroy(&dev->sem);
		os_spinlock_destroy(&dev->spinlock);
		os_pci_master_clear(&dev->pci);
		os_pci_dev_disable(&dev->pci);
	}
}


