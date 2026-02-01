// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_init.c $
// $Rev: 50967 $
// $Date: 2022-04-25 08:41:30 -0500 (Mon, 25 Apr 2022) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



// variables ******************************************************************

gsc_global_t	gsc_global;



/******************************************************************************
*
*	Function:	_check_id
*
*	Purpose:
*
*		Check to see if the referenced device is one we support. If it is, then
*		fill in identity fields for the device structure given.
*
*	Arguments:
*
*		dev		The device structure were we put identify data.
*
*	Returned:
*
*		0		We don't support this device.
*		1		We do support this device.
*
******************************************************************************/

static int _check_id(dev_data_t* dev)
{
	u16	didr;
	int	found	= 0;
	int	i;
	u16	sdidr;
	u16	svidr;
	u16	vidr;

	vidr	= os_reg_pci_rx_u16(dev, 0, 0x00);
	didr	= os_reg_pci_rx_u16(dev, 0, 0x02);
	svidr	= os_reg_pci_rx_u16(dev, 0, 0x2C);
	sdidr	= os_reg_pci_rx_u16(dev, 0, 0x2E);

	for (i = 0; dev_id_list[i].model; i++)
	{
		if (dev_id_list[i].vendor != vidr)
			continue;

		if (dev_id_list[i].device != didr)
			continue;

		if ((dev_id_list[i].sub_vendor != svidr)	&&
			(dev_id_list[i].sub_vendor != -1))
		{
			continue;
		}

		if ((dev_id_list[i].sub_device != sdidr)	&&
			(dev_id_list[i].sub_device != -1))
		{
			continue;
		}

		found			= 1;
		dev->model		= dev_id_list[i].model;
		dev->board_type	= dev_id_list[i].type;
		break;

		// It is possible for the model and type to change once the device
		// specific code has a chance to examine the board more closely.
	}

#if DEV_PCI_ID_SHOW
	{
		char	buf[128];

		if (found)
			sprintf(buf, " <--- %s, type %d", dev->model, dev->board_type);
		else
			buf[0]	= 0;

		printf(	"ID: %04lX %04lX %04lX %04lX%s\n",
				(long) vidr,
				(long) didr,
				(long) svidr,
				(long) sdidr,
				buf);
	}
#endif

	return(found);
}



/******************************************************************************
*
*	Function:	_dev_data_t_add
*
*	Purpose:
*
*		Add a device to our device list.
*
*	Arguments:
*
*		dev		A pointer to the structure to add.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

static int _dev_data_t_add(dev_data_t* dev)
{
	int	ret;
	int	i;

	for (;;)	// A convenience loop.
	{
		for (i = 0; i < (int) SIZEOF_ARRAY(gsc_global.dev_list); i++)
		{
			if (gsc_global.dev_list[i] == NULL)
				break;
		}

		if (i >= (int) SIZEOF_ARRAY(gsc_global.dev_list))
		{
			// We don't have enough room to add the device.
			ret	= -ENOMEM;
			printf("%s: Too many devices were found.\n", DEV_NAME);
			break;
		}

		gsc_global.dev_list[i]	= dev;
		dev->board_index		= i;
		ret						= 0;
		gsc_global.dev_qty++;
		break;
	}

	return(ret);
}



//*****************************************************************************
int gsc_init_dev_create(dev_data_t* dev)
{
	int	found;
	int	ret;

	for (;;)	// A convenience loop.
	{
		ret	= os_open_pre_access(dev);

		if (ret)
		{
			// We're not able to access the device.
			ret	= 0;
			break;
		}

		found	= _check_id(dev);

		if (found == 0)
		{
			// The device is not ours.
			break;
		}

		ret	= dev_device_create(dev);

		if (ret == 0)
			ret	= gsc_endian_init(dev);

		if (ret == 0)
			ret	= gsc_eeprom_access(dev);

		if (ret < 0)
		{
			// There was a problem.
		}
		else
		{
			// It is our device and setup was successful.
			ret	= 1;
		}

		break;
	}

	return(ret);
}



//*****************************************************************************
void gsc_init_dev_destroy(dev_data_t* dev)
{
	if (dev)
		dev_device_destroy(dev);
}



//*****************************************************************************
int gsc_init_dev_data_t_alloc(dev_data_t** dev)
{
	int	ret;
	int	size	= sizeof(dev_data_t);

	for (;;)	// A convenience loop.
	{
		if (dev == NULL)
		{
			ret	= -EINVAL;
			break;
		}

		dev[0]	= os_mem_data_alloc(size);

		if (dev[0] == NULL)
		{
			ret	= -EINVAL;
			break;
		}

		ret	= _dev_data_t_add(dev[0]);

		if (ret)
		{
			// There was a problem.
			os_mem_data_free(dev[0]);
			dev[0]	= NULL;
		}

		break;
	}

	return(ret);
}



//*****************************************************************************
void gsc_init_dev_data_t_free(dev_data_t** dev)
{
	int	i;

	if ((dev) && (dev[0]))
	{
		for (i = 0; i < (int) SIZEOF_ARRAY(gsc_global.dev_list); i++)
		{
			if (gsc_global.dev_list[i] == dev[0])
			{
				gsc_global.dev_list[i]	= NULL;
				gsc_global.dev_qty--;
				break;
			}
		}

		os_mem_data_free(dev[0]);
		dev[0]	= NULL;
	}
}



