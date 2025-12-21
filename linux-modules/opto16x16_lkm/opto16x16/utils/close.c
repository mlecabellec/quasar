// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/close.c $
// $Rev: 53726 $
// $Date: 2023-09-14 10:45:45 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_close_util
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
*		verbose	Work verbosely?
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int opto16x16_close_util(int fd, int index, int verbose)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Close", index);

	ret		= opto16x16_close(fd);
	errs	= (ret < 0) ? 1 : 0;

	if (verbose == 0)
		;
	else if (errs)
		printf("FAIL <---  (ret = %d)\n", ret);
	else
		printf("PASS\n");

	return(errs);
}


