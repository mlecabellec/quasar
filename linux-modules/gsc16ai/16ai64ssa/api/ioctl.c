// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/api/ioctl.c $
// $Rev: 54952 $
// $Date: 2024-08-07 15:23:29 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	0	= success
//	<0  = failure, value is -errno or -GetLastError (Windows only)
//	>0	= never returned
int ai64ssa_ioctl(int fd, int request, void* arg)
{
	int	ret;

	ret	= gsc_api_ioctl(fd, request, arg);
	return(ret);
}


