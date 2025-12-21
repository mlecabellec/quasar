// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_eeprom.c $
// $Rev: 51062 $
// $Date: 2022-05-31 10:22:49 -0500 (Tue, 31 May 2022) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



//*****************************************************************************
int gsc_eeprom_access(dev_data_t* dev)
{
#ifdef DEV_EEPROM_ACCESS
	int	ret;

	ret	= gsc_eeprom_access_pci(dev);
	return(ret);
#else
	return(0);
#endif
}


