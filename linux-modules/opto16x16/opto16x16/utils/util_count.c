// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_count.c $
// $Rev: 53726 $
// $Date: 2023-09-14 10:45:45 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_count_boards
*
*	Purpose:
*
*		Count the number of installed devices.
*
*	Arguments:
*
*		verbose	Work verbosely?
*
*		get		Report the count here.
*
*	Returned:
*
*		> 0		The number of devices found.
*		== 0	No devices were found.
*
******************************************************************************/

int opto16x16_count_boards(int verbose, s32* get)
{
	int	errs;

	errs	= os_count_boards(verbose, get, opto16x16_open, opto16x16_read, opto16x16_close);
	return(errs);
}



