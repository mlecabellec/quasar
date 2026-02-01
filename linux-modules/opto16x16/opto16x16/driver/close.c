// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/driver/close.c $
// $Rev: 47358 $
// $Date: 2020-06-02 12:09:05 -0500 (Tue, 02 Jun 2020) $

// OPTO16X16: Device Driver: source file

#include "main.h"



//*****************************************************************************
int dev_close(dev_data_t* dev)
{
	initialize_ioctl(dev, NULL);
	gsc_wait_close(dev);
	gsc_irq_close(dev, 0);
	return(0);
}


