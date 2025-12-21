// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/close.c $
// $Rev: 43470 $
// $Date: 2018-08-31 13:40:17 -0500 (Fri, 31 Aug 2018) $

// 16AI64SSA: Device Driver: source file

#include "main.h"



//*****************************************************************************
int dev_close(dev_data_t* dev)
{
	initialize_ioctl(dev, NULL);
	gsc_wait_close(dev);
	dev_io_close(dev);
	gsc_dma_close(dev, 0);
	gsc_irq_close(dev, 0);
	return(0);
}



