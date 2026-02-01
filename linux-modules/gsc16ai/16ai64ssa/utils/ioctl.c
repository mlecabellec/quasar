// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/ioctl.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_ioctl_util
*
*	Purpose:
*
*		Implement a visual wrapper around the device IOCTL call.
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
*		request	The IOCTL service to request.
*
*		arg		The argument for the IOCTL service.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int ai64ssa_ioctl_util(int fd, int index, int verbose, int request, void* arg)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("IOCTL", index);

	ret		= ai64ssa_ioctl(fd, request, arg);
	errs	= (ret < 0) ? 1 : 0;

	if (verbose == 0)
		;
	else if (errs)
		printf("FAIL <---  (#%d)\n", (int) OS_IOCTL_INDEX_DECODE(request));
	else
		printf("PASS  (#%d)\n", (int) OS_IOCTL_INDEX_DECODE(request));

	return(errs);
}


