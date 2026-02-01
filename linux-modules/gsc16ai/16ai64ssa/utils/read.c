// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/read.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_read_util
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
*		verbose	Work verbosely? 0 = no, !0 = yes
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

int ai64ssa_read_util(int fd, int index, int verbose, void* dst, size_t bytes, size_t* got)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Reading", index);

	ret	= ai64ssa_read(fd, dst, bytes);
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


