// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_input_sync.c $
// $Rev: 42829 $
// $Date: 2018-05-17 17:02:12 -0500 (Thu, 17 May 2018) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_input_sync
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_INPUT_SYNC service.
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
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int	ai64ssa_input_sync(int fd, int index, int verbose)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Input Sync", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_INPUT_SYNC, NULL);
	errs	= ret ? 1 : 0;


	if (verbose)
		printf("%s\n", errs ? "FAIL <---" : "PASS");

	return(errs);
}


