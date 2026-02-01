// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/ioctl.c $
// $Rev: 53726 $
// $Date: 2023-09-14 10:45:45 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_ioctl_util
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
*		verbose	Work verbosely?
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

int opto16x16_ioctl_util(int fd, int index, int verbose, int request, void* arg)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("IOCTL", index);

	ret		= opto16x16_ioctl(fd, request, arg);
	errs	= (ret < 0) ? 1 : 0;

	if (verbose == 0)
		;
	else if (errs)
		printf("FAIL <---  (#%d)\n", (int) OS_IOCTL_INDEX_DECODE(request));
	else
		printf("PASS  (#%d)\n", (int) OS_IOCTL_INDEX_DECODE(request));

	return(errs);
}


