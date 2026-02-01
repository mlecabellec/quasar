// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/open.c $
// $Rev: 53726 $
// $Date: 2023-09-14 10:45:45 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_open_util
*
*	Purpose:
*
*		Implement a visual wrapper around the device open call.
*
*	Arguments:
*
*		device	The zero based index of the device to access.
*
*		share	Open the device in shared access mode?
*
*		index	The index of the device to access. Ignore if < 0.
*				This is for display purposes only.
*
*		verbose	Work verbosely?
*
*		fd		The file descriptor is returned here.
*
*	Returned:
*
*		0		All went well. The fd value is valid.
*		> 0		The number of errors seen. The fd value is -1.
*
******************************************************************************/

int opto16x16_open_util(int device, int share, int index, int verbose, int* fd)
{
	const char*	access	= share ? "Share" : "Exclusive";
	int			errs;
	int			ret;

	if (verbose)
		gsc_label_index("Open", index);

	ret		= opto16x16_open(device, share, fd);
	errs	= (ret < 0) ? 1 : 0;

	if (verbose == 0)
		;
	else if (errs)
		printf("FAIL <---  (%s, ret = %d)\n", access, ret);
	else
		printf("PASS  (%s)\n", access);

	return(errs);
}


