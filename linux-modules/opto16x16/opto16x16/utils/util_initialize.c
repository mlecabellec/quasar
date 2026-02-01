// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_initialize.c $
// $Rev: 44253 $
// $Date: 2018-12-10 12:58:41 -0600 (Mon, 10 Dec 2018) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_initialize
*
*	Purpose:
*
*		Provide a visual wrapper for the OPTO16X16_IOCTL_INITIALIZE service.
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

int opto16x16_initialize(int fd, int index, int verbose)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Initialize", index);

	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_INITIALIZE, NULL);
	errs	= ret ? 1 : 0;

	if (verbose)
		printf("%s\n", errs ? "FAIL <---" : "PASS");

	return(errs);
}


