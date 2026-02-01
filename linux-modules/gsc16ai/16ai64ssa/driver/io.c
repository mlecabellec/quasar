// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/io.c $
// $Rev: 43470 $
// $Date: 2018-08-31 13:40:17 -0500 (Fri, 31 Aug 2018) $

// 16AI64SSA: Device Driver: source file

#include "main.h"



/******************************************************************************
*
*	Function:	dev_io_close
*
*	Purpose:
*
*		Cleanup the I/O stuff for the device as it is being closed.
*
*	Arguments:
*
*		dev	The data for the device of interest.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void dev_io_close(dev_data_t* dev)
{
	int	i;

	for (i = 0; i < DEV_IO_STREAM_QTY; i++)
	{
		if (dev->io.io_streams[i])
		{
			if (dev->io.io_streams[i]->dev_io_close)
			{
				(dev->io.io_streams[i]->dev_io_close)(dev, dev->io.io_streams[i]);
			}
		}
	}
}



/******************************************************************************
*
*	Function:	dev_io_create
*
*	Purpose:
*
*		Perform I/O based initialization as the driver is being loaded.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*	Returned:
*
*		0		All went well.
*		< 0		The error status for the problem encounterred.
*
******************************************************************************/

int dev_io_create(dev_data_t* dev)
{
	int	ret;

	dev->io.io_streams[0]	= &dev->io.rx;

	ret	= dev_read_create(dev, &dev->io.rx);

	return(ret);
}



/******************************************************************************
*
*	Function:	dev_io_destroy
*
*	Purpose:
*
*		Perform I/O based cleanup as the driver is being unloaded.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void dev_io_destroy(dev_data_t* dev)
{
	int	i;

	dev_io_close(dev);	// Just in case.

	for (i = 0; i < DEV_IO_STREAM_QTY; i++)
	{
		if (dev->io.io_streams[i])
		{
			gsc_io_destroy(dev, dev->io.io_streams[i]);
		}
	}
}



/******************************************************************************
*
*	Function:	dev_io_open
*
*	Purpose:
*
*		Perform I/O based initialization as the device is being opened.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*	Returned:
*
*		0		All went well.
*		< 0		The code for the error seen.
*
******************************************************************************/

int dev_io_open(dev_data_t* dev)
{
	int	i;

	dev_io_close(dev);	// Just in case.

	for (i = 0; i < DEV_IO_STREAM_QTY; i++)
	{
		if (dev->io.io_streams[i])
		{
			if (dev->io.io_streams[i]->dev_io_open)
				(dev->io.io_streams[i]->dev_io_open)(dev, dev->io.io_streams[i]);
		}
	}

	return(0);
}



/******************************************************************************
*
*	Function:	dev_io_read_select
*
*	Purpose:
*
*		Select the dev_io_t structure for the referenced I/O read stream.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*		count	The number of bytes to transfer, with the I/O selector encoded
*				in the 0xF0000000 nibble.
*
*	Returned:
*
*		0		All went well.
*		< 0		The code for the error seen.
*
******************************************************************************/

dev_io_t* dev_io_read_select(dev_data_t* dev, size_t select)
{
	dev_io_t*	io;

	// We have only one input stream.
	io	= &dev->io.rx;
	return(io);
}



