// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/reg.c $
// $Rev: 43470 $
// $Date: 2018-08-31 13:40:17 -0500 (Fri, 31 Aug 2018) $

// 16AI64SSA: Device Driver: source file

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


