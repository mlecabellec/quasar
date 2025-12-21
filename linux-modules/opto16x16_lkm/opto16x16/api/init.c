// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/api/init.c $
// $Rev: 43069 $
// $Date: 2018-06-22 18:02:53 -0500 (Fri, 22 Jun 2018) $

// OPTO16X16: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	0  = success
//	<0 = failure, value is -errno or -GetLastError (Windows only)
//	>0 = never returned
int opto16x16_init(void)
{
	int	ret;

	ret	= gsc_api_init();
	return(ret);
}


