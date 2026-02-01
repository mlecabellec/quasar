// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/open.c $
// $Rev: 43470 $
// $Date: 2018-08-31 13:40:17 -0500 (Fri, 31 Aug 2018) $

// 16AI64SSA: Device Driver: source file

#include "main.h"



//*****************************************************************************
int dev_open(dev_data_t* dev)
{
	int	ret;

	ret	= dev_io_open(dev);

	if (ret == 0)
		ret	= gsc_dma_open(dev, 0);

	if (ret == 0)
		ret	= gsc_irq_open(dev, 0);

	if (ret == 0)
		initialize_ioctl(dev, NULL);

	if (ret)
		dev_close(dev);

	return(ret);
}


