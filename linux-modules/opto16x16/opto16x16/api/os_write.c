// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/api/linux/os_write.c $
// $Rev: 42938 $
// $Date: 2018-06-01 11:01:37 -0500 (Fri, 01 Jun 2018) $

// Linux: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	>=0 = success
//	<0  = failure, value is -errno
int os_api_write(int fd, const void* src, size_t bytes)
{
	int	ret;

	if (os_api_global.init == 0)
	{
		ret	= -EPROTO;
	}
	else
	{
		fd	= FD_DECODE(fd);
		ret	= write(fd, src, bytes);

		if (ret < 0)
			ret	= -errno;
	}

	return(ret);
}


