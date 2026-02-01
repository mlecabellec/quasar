// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/driver/open.c $
// $Rev: 47358 $
// $Date: 2020-06-02 12:09:05 -0500 (Tue, 02 Jun 2020) $

// OPTO16X16: Device Driver: source file

#include "main.h"



//*****************************************************************************
int dev_open(dev_data_t* dev)
{
	int	ret;

	ret	= gsc_irq_open(dev, 0);

	if (ret == 0)
		initialize_ioctl(dev, NULL);

	if (ret)
		dev_close(dev);

	return(ret);
}


