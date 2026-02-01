// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_auto_calibrate.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



// This source is retained for backwards compatibility only.

/******************************************************************************
*
*	Function:	ai64ssa_auto_calibrate
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_AUTO_CALIBRATE service.
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

int	ai64ssa_auto_calibrate(int fd, int index, int verbose)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Autocalibrate", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_AUTO_CALIBRATE, NULL);
	errs	= ret ? 1 : 0;

	if (verbose)
		printf("%s\n", errs ? "FAIL <---" : "PASS");

	return(errs);
}


