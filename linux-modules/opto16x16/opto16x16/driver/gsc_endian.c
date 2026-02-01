// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_endian.c $
// $Rev: 50967 $
// $Date: 2022-04-25 08:41:30 -0500 (Mon, 25 Apr 2022) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



//*****************************************************************************
int gsc_endian_init(dev_data_t* dev)
{
	int	ret;

	ret	= gsc_endian_init_pci(dev);
	return(ret);
}


