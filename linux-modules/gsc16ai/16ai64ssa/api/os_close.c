// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/api/linux/os_close.c $
// $Rev: 42938 $
// $Date: 2018-06-01 11:01:37 -0500 (Fri, 01 Jun 2018) $

// Linux: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	0  = success
//	<0 = failure, value is -errno
//	>0 = never returned
int os_api_close(int fd)
{
	int	ret;

	if (os_api_global.init == 0)
	{
		ret	= -EPROTO;
	}
	else
	{
		fd	= FD_DECODE(fd);
		ret	= close(fd);

		if (ret < 0)
			ret	= -errno;
		else
			ret	= 0;
	}

	return(ret);
}


