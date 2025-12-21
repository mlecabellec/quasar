// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/api/ioctl.c $
// $Rev: 53730 $
// $Date: 2023-09-14 10:48:31 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	0	= success
//	<0  = failure, value is -errno or -GetLastError (Windows only)
//	>0	= never returned
int opto16x16_ioctl(int fd, int request, void* arg)
{
	int	ret;

	ret	= gsc_api_ioctl(fd, request, arg);
	return(ret);
}


