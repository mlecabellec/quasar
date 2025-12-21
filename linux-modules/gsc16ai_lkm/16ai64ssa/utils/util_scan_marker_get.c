// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_scan_marker_get.c $
// $Rev: 42829 $
// $Date: 2018-05-17 17:02:12 -0500 (Thu, 17 May 2018) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_scan_marker_get
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_SCAN_MARKER_GET service.
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
*		get		The current setting is recorded here, if not NULL.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int	ai64ssa_scan_marker_get(int fd, int index, int verbose, u32* get)
{
	int	errs;
	int	ret;
	u32	set;

	if (verbose)
		gsc_label_index("Scan Marker Value", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_SCAN_MARKER_GET, &set);
	errs	= ret ? 1 : 0;

	if (verbose)
		printf("%s  (retrived 0x%08lX)\n", errs ? "FAIL <---" : "PASS", (long) set);

	if (get)
		get[0]	= set;

	return(errs);
}


