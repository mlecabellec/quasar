// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/init.c $
// $Rev: 53726 $
// $Date: 2023-09-14 10:45:45 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_init_util
*
*	Purpose:
*
*		Implement a visual wrapper around the API init call.
*
*	Arguments:
*
*		verbose	Work verbosely?
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int opto16x16_init_util(int verbose)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label("API Initialize");

	ret		= opto16x16_init();
	errs	= (ret < 0) ? 1 : 0;

	if (verbose == 0)
		;
	else if (errs)
		printf("FAIL <---  (ret = %d)\n", ret);
	else
		printf("PASS\n");

	return(errs);
}



