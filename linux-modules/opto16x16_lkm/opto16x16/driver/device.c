// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/driver/device.c $
// $Rev: 51408 $
// $Date: 2022-07-14 14:22:37 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Device Driver: source file

#include "main.h"



// variables ******************************************************************

const gsc_dev_id_t	dev_id_list[]	=
{
	// model	Vendor	Device	SubVen	SubDev	type

	{ "OPTO16X16",	0x10B5, 0x9056, 0x10B5, 0x3460,	GSC_DEV_TYPE_OPTO16X16	},
	{ NULL }
};



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

	int	ret;

	for (;;)	// A convenience loop.
	{
		// Verify some macro contents.
		ret	= gsc_macro_test_base_name(OPTO16X16_BASE_NAME);
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

		// Firmware access.
		dev->vaddr.gsc_bcsr_8	= GSC_VADDR(dev, GSC_REG_OFFSET(OPTO16X16_GSC_BCSR));
		dev->vaddr.gsc_cdr_32	= GSC_VADDR(dev, GSC_REG_OFFSET(OPTO16X16_GSC_CDR));
		dev->vaddr.gsc_cier_16	= GSC_VADDR(dev, GSC_REG_OFFSET(OPTO16X16_GSC_CIER));
		dev->vaddr.gsc_cosr_16	= GSC_VADDR(dev, GSC_REG_OFFSET(OPTO16X16_GSC_COSR));
		dev->vaddr.gsc_cpr_16	= GSC_VADDR(dev, GSC_REG_OFFSET(OPTO16X16_GSC_CPR));
		dev->vaddr.gsc_odr_16	= GSC_VADDR(dev, GSC_REG_OFFSET(OPTO16X16_GSC_ODR));
		dev->vaddr.gsc_recr_16	= GSC_VADDR(dev, GSC_REG_OFFSET(OPTO16X16_GSC_RECR));
		dev->vaddr.gsc_rdr_16	= GSC_VADDR(dev, GSC_REG_OFFSET(OPTO16X16_GSC_RDR));

		// Data cache initialization.

		// Initialize additional resources.
		ret	= dev_irq_create(dev);
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
		dev_irq_destroy(dev);
		gsc_bar_destroy(&dev->bar);
		os_sem_destroy(&dev->sem);
		os_spinlock_destroy(&dev->spinlock);
		os_pci_master_clear(&dev->pci);
		os_pci_dev_disable(&dev->pci);
	}
}


