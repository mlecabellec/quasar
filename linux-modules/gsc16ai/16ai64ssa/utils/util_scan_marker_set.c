// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_scan_marker_set.c $
// $Rev: 42829 $
// $Date: 2018-05-17 17:02:12 -0500 (Thu, 17 May 2018) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_scan_marker_set
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_SCAN_MARKER_SET service.
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
*		set		The setting to apply.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int	ai64ssa_scan_marker_set(int fd, int index, int verbose, u32 set)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Scan Marker Value", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_SCAN_MARKER_SET, &set);
	errs	= ret ? 1 : 0;


	if (verbose)
		printf("%s  (Set to 0x%08lX)\n", errs ? "FAIL <---" : "PASS", (long) set);

	return(errs);
}


