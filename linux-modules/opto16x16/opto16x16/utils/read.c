// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/read.c $
// $Rev: 53726 $
// $Date: 2023-09-14 10:45:45 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_read_util
*
*	Purpose:
*
*		Implement a visual wrapper around the device read call.
*
*	Arguments:
*
*		fd		Use this handle to access the device.
*
*		index	The index of the device to access. Ignore if < 0.
*				This is for display purposes only.
*
*		verbose	Work verbosely?
*
*		dst		The data read is placed here.
*
*		bytes	The volume of data to read.
*
*		got		If non-NULL, then this is the number of bytes received.
*
*	Returned:
*
*		>= 0	The volume of data read.
*		< 0		The negative of the number of errors seen.
*
******************************************************************************/

int opto16x16_read_util(int fd, int index, int verbose, void* dst, size_t bytes, size_t* got)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Reading", index);

	ret	= opto16x16_read(fd, dst, bytes);
	errs	= (ret < 0) ? 1 : 0;

	if (verbose == 0)
	{
	}
	else if (ret >= 0)
	{
		printf("PASS  (%d Byte%s)\n", ret, (ret == 1) ? "" : "s");
	}
	else
	{
		printf(	"FAIL <---  (requested %ld Byte%s, ret = %ld)\n",
				(long) bytes,
				(bytes == 1) ? "" : "s",
				(long) ret);
	}

	if (got)
		got[0]	= (ret >= 0) ? ret : 0;

	return(errs);
}


