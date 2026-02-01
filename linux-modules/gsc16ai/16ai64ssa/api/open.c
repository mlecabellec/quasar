// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/api/open.c $
// $Rev: 43042 $
// $Date: 2018-06-22 17:50:23 -0500 (Fri, 22 Jun 2018) $

// 16AI64SSA: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	0	= success, fd is valid and >= 0
//	<0  = failure, value is -errno or -GetLastError (Windows only), fd is -1
//	>0	= never returned
int ai64ssa_open(int device, int share, int* fd)
{
	int	ret;

	ret	= gsc_api_open(device, share, fd, AI64SSA_BASE_NAME);
	return(ret);
}


