// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/close.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_close_util
*
*	Purpose:
*
*		Implement a visual wrapper around the device close call.
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
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int ai64ssa_close_util(int fd, int index, int verbose)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Close", index);

	ret		= ai64ssa_close(fd);
	errs	= (ret < 0) ? 1 : 0;

	if (verbose == 0)
		;
	else if (errs)
		printf("FAIL <---  (ret = %d)\n", ret);
	else
		printf("PASS\n");

	return(errs);
}


