// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/api/linux/os_open.c $
// $Rev: 50379 $
// $Date: 2022-02-16 16:44:49 -0600 (Wed, 16 Feb 2022) $

// Linux: API Library: source file

#include "main.h"



// macros *********************************************************************

#ifndef	OS_DEV_NAME_SEPARATOR
#define	OS_DEV_NAME_SEPARATOR		"."
#endif



//*****************************************************************************
// Return Values:
//	>=0 = success
//	<0  = failure, value is -errno
int os_api_open(int device, int share, const char* base_name)
{
	int		fd;
	int		flags;
	char	name[128];

	if (os_api_global.init == 0)
	{
		fd	= -EPROTO;
	}
	else if (device == -1)
	{
		sprintf(name, "/proc/%s", base_name);
		fd	= open(name, S_IRUSR);

		if (fd < 0)
			fd	= -errno;
		else
			fd	= FD_ENCODE(fd);
	}
	else
	{
		sprintf(name, "/dev/%s%s%d", base_name, OS_DEV_NAME_SEPARATOR, device);
		flags	= O_RDWR | (share ? O_APPEND : 0);
		fd		= open(name, flags);

		if (fd < 0)
			fd	= -errno;
		else
			fd	= FD_ENCODE(fd);
	}

	return(fd);
}


