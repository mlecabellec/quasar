// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/driver/reg.c $
// $Rev: 44249 $
// $Date: 2018-12-10 11:09:26 -0600 (Mon, 10 Dec 2018) $

// OPTO16X16: Device Driver: source file

#include "main.h"



//*****************************************************************************
int dev_reg_mod_alt(dev_data_t* dev, gsc_reg_t* arg)
{
	// No alternate register encodings are defined for this driver.
	return(-EINVAL);
}



//*****************************************************************************
int dev_reg_read_alt(dev_data_t* dev, gsc_reg_t* arg)
{
	// No alternate register encodings are defined for this driver.
	return(-EINVAL);
}



//*****************************************************************************
int dev_reg_write_alt(dev_data_t* dev, gsc_reg_t* arg)
{
	// No alternate register encodings are defined for this driver.
	return(-EINVAL);
}


