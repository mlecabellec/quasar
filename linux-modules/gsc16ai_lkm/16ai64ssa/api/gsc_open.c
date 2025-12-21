// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/api/gsc_open.c $
// $Rev: 45870 $
// $Date: 2019-08-13 12:51:17 -0500 (Tue, 13 Aug 2019) $

// OS & Device Independent: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	0	= success, fd is valid and >= 0
//	<0  = failure, value is -errno or -GetLastError (Windows only), fd is -1
//	>0	= never returned
int gsc_api_open(int device, int share, int* fd, const char* base_name)
{
	int	ret;

	if (fd == NULL)
	{
		ret	= -EINVAL;
	}
	else
	{
		ret		= os_api_open(device, share, base_name);
		fd[0]	= (ret < 0) ? -1 : ret;
		ret		= (ret < 0) ? ret : 0;
	}

	return(ret);
}


