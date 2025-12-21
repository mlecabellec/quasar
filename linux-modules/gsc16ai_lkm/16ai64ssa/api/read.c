// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/api/read.c $
// $Rev: 44778 $
// $Date: 2019-05-02 13:44:41 -0500 (Thu, 02 May 2019) $

// 16AI64SSA: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	>=0	= success, the number of bytes transferred
//	<0  = failure, value is -errno or -GetLastError (Windows only)
int ai64ssa_read(int fd, void* dst, size_t bytes)
{
	int	ret;

	if (bytes > GSC_IO_SIZE_QTY_MASK)
		bytes	= GSC_IO_SIZE_QTY_MASK & ~0x3;

	bytes	|= STREAM_ID_RX;
	ret		= gsc_api_read(fd, dst, bytes);
	return(ret);
}


