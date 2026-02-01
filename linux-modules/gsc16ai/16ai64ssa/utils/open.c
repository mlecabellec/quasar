// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/open.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_open_util
*
*	Purpose:
*
*		Implement a visual wrapper around the device open call.
*
*	Arguments:
*
*		device	The zero based index of the device to access.
*
*		share	Open the device in shared access mode?
*
*		index	The index of the device to access. Ignore if < 0.
*				This is for display purposes only.
*
*		verbose	Work verbosely? 0 = no, !0 = yes
*
*		fd		The file descriptor is returned here.
*
*	Returned:
*
*		0		All went well. The fd value is valid.
*		> 0		The number of errors seen. The fd value is -1.
*
******************************************************************************/

int ai64ssa_open_util(int device, int share, int index, int verbose, int* fd)
{
	const char*	access	= share ? "Share" : "Exclusive";
	int			errs;
	int			ret;

	if (verbose)
		gsc_label_index("Open", index);

	ret		= ai64ssa_open(device, share, fd);
	errs	= (ret < 0) ? 1 : 0;

	if (verbose == 0)
		;
	else if (errs)
		printf("FAIL <---  (%s, ret = %d)\n", access, ret);
	else
		printf("PASS  (%s)\n", access);

	return(errs);
}


