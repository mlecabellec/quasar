// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_wait_cancel.c $
// $Rev: 53726 $
// $Date: 2023-09-14 10:45:45 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_wait_cancel
*
*	Purpose:
*
*		Provide a visual wrapper for the OPTO16X16_IOCTL_WAIT_CANCEL service.
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

int opto16x16_wait_cancel(int fd, int index, int verbose, gsc_wait_t* wait)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Wait Cancel", index);

	if (wait)
	{
		ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_WAIT_CANCEL, wait);

		if (verbose)
		{
			printf(	"%s  (%ld wait%s cancelled)\n",
					ret ? "FAIL <---" : "PASS",
					(long) wait->count,
					(wait->count == 1) ? "" : "s");
		}
	}
	else
	{
		ret		= 1;

		if (verbose)
			printf(	"FAIL <---  (NULL pointer)\n");
	}

	errs	= ret ? 1 : 0;
	return(errs);
}


