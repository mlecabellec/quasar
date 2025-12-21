// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/api/close.c $
// $Rev: 43042 $
// $Date: 2018-06-22 17:50:23 -0500 (Fri, 22 Jun 2018) $

// 16AI64SSA: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	0	= success
//	<0  = failure, value is -errno or -GetLastError (Windows only)
//	>0	= never returned
int ai64ssa_close(int fd)
{
	int	ret;

	ret	= gsc_api_close(fd);
	return(ret);
}


