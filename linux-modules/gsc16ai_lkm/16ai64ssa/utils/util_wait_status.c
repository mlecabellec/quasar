// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_wait_status.c $
// $Rev: 42829 $
// $Date: 2018-05-17 17:02:12 -0500 (Thu, 17 May 2018) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_wait_status
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_WAIT_STATUS service.
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
*		wait	This is the crieteria to use.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int ai64ssa_wait_status(int fd, int index, int verbose, gsc_wait_t* wait)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Wait Status", index);

	if (wait)
	{
		ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_WAIT_STATUS, wait);
		errs	= ret ? 1 : 0;

		if (verbose)
		{
			printf(	"%s  (%ld matching wait%s)\n",
					errs ? "FAIL <---" : "PASS",
					(long) wait->count,
					(wait->count == 1) ? "" : "s");
		}
	}
	else
	{
		errs	= 1;

		if (verbose)
			printf(	"FAIL <---  (NULL pointer)\n");
	}

	return(errs);
}


