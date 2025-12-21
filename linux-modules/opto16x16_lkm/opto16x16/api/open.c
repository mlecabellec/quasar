// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/api/open.c $
// $Rev: 43069 $
// $Date: 2018-06-22 18:02:53 -0500 (Fri, 22 Jun 2018) $

// OPTO16X16: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	0	= success, fd is valid and >= 0
//	<0  = failure, value is -errno or -GetLastError (Windows only), fd is -1
//	>0	= never returned
int opto16x16_open(int device, int share, int* fd)
{
	int	ret;

	ret	= gsc_api_open(device, share, fd, OPTO16X16_BASE_NAME);
	return(ret);
}


