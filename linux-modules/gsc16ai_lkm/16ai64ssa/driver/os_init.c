// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_init.c $
// $Rev: 53921 $
// $Date: 2023-11-28 16:48:39 -0600 (Tue, 28 Nov 2023) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



/******************************************************************************
*
*	Function:	_cleanup_module
*
*	Purpose:
*
*		Clean things up when the kernel is about to unload the module.
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

static void _cleanup_module(void)
{
	dev_data_t*	dev;
	int			i;

	gsc_global.driver_unloading	= 1;
	printf(	"%s: driver unloading: version %s\n",
			DEV_NAME,
			GSC_DRIVER_VERSION);

	if (gsc_global.major_number >= 0)
	{
		unregister_chrdev(gsc_global.major_number, DEV_NAME);
		gsc_global.major_number	= -1;
	}

	for (i = 0; i < (int) SIZEOF_ARRAY(gsc_global.dev_list); i++)
	{
		if (gsc_global.dev_list[i])
		{
			dev	= gsc_global.dev_list[i];
			gsc_init_dev_destroy(dev);
			gsc_init_dev_data_t_free(&dev);
		}
	}

	gsc_ioctl_reset();
	os_proc_stop();
	gsc_global.dev_qty	= 0;

	if (gsc_global.driver_loaded)
	{
		gsc_global.driver_loaded	= 0;
		printf(	"%s: driver unloaded: version %s\n",
				DEV_NAME,
				GSC_DRIVER_VERSION);
	}

	gsc_global.driver_unloading	= 0;
}



/******************************************************************************
*
*	Function:	_init_module
*
*	Purpose:
*
*		Initialize the driver upon loading.
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

static int _init_module(void)
{
	dev_data_t*		dev;
	struct pci_dev*	pci		= NULL;
	int				ret		= 0;

	gsc_global.major_number	= -1;
	printf(	"%s: driver loading: version %s\n",
			DEV_NAME,
			GSC_DRIVER_VERSION);

	// Locate the devices and add them to our list.

	PCI_DEVICE_LOOP(pci)
	{
		ret	= gsc_init_dev_data_t_alloc(&dev);

		if (ret < 0)
		{
			printf(	"%s: %d. %s: structure allocation failed.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		if (dev == NULL)
		{
			// This should never happen!
			ret	= -EFAULT;
			printf(	"%s: %d. %s: 'dev' is NULL.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		dev->pci.pd	= pci;
		ret			= gsc_init_dev_create(dev);

		if (ret > 0)
		{
			// The referenced device was added.
			printf("%s: device loaded: %s\n", DEV_NAME, dev->model);
			dev	= NULL;
			ret	= 0;
		}
		else if (ret == 0)
		{
			// The referenced device was not ours.
			gsc_init_dev_destroy(dev);
			gsc_init_dev_data_t_free(&dev);
		}
		else	// ret < 0
		{
			// The referenced device was not added.
			// An error should have been reported.
			gsc_init_dev_destroy(dev);
			gsc_init_dev_data_t_free(&dev);
			printf(	"%s: %d. %s: device addition failed.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			ret	= 0;
		}
	}

	// Perform initialization following device discovery.

	for (;;)	// A convenience loop.
	{
		if (gsc_global.dev_qty <= 0)
		{
			ret	= -ENODEV;
			_cleanup_module();
			printf(	"%s: driver load failure: version %s\n",
					DEV_NAME,
					GSC_DRIVER_VERSION);
			break;
		}

		gsc_global.fops.open	= os_open;
		gsc_global.fops.release	= os_close;
		gsc_global.fops.read	= os_read;
		gsc_global.fops.write	= os_write;
		IOCTL_SET_BKL(&gsc_global.fops, os_ioctl_bkl);
		IOCTL_SET_COMPAT(&gsc_global.fops, os_ioctl_compat);
		IOCTL_SET_UNLOCKED(&gsc_global.fops, os_ioctl_unlocked);
		SET_MODULE_OWNER(&gsc_global.fops);

		gsc_global.major_number	= register_chrdev(	0,
													DEV_NAME,
													&gsc_global.fops);

		if (gsc_global.major_number < 0)
		{
			ret	= -ENODEV;
			printf(	"%s: %d. %s: register_chrdev failed.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
		}

		if (ret == 0)
			ret	= os_proc_start();

		if (ret == 0)
			ret	= gsc_ioctl_init();

		if (ret)
		{
			_cleanup_module();
			break;
		}

		gsc_global.driver_loaded	= 1;
		printf(	"%s: driver loaded: version %s\n",
				DEV_NAME,
				GSC_DRIVER_VERSION);
		ret	= 0;
		break;
	}

	return(ret);
}



// macros *********************************************************************

module_init(_init_module);
module_exit(_cleanup_module);


