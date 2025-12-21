// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/api/read.c $
// $Rev: 45830 $
// $Date: 2019-08-12 17:23:45 -0500 (Mon, 12 Aug 2019) $

// OPTO16X16: API Library: source file

#include "main.h"



//*****************************************************************************
// This is for reading information from the device driver.
// Return Values:
//	>=0	= success, the number of bytes transferred
//	<0  = failure, value is -errno or -GetLastError (Windows only)
int opto16x16_read(int fd, void* dst, size_t bytes)
{
	int	ret;

	if (bytes > GSC_IO_SIZE_QTY_MASK)
		bytes	= GSC_IO_SIZE_QTY_MASK & ~0x3;

	bytes	|= STREAM_ID_RX;
	ret		= gsc_api_read(fd, dst, bytes);
	return(ret);
}


