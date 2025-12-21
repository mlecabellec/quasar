// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/init.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_init_util
*
*	Purpose:
*
*		Implement a visual wrapper around the API init call.
*
*	Arguments:
*
*		verbose	Work verbosely? 0 = no, !0 = yes
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int ai64ssa_init_util(int verbose)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label("API Initialize");

	ret		= ai64ssa_init();
	errs	= (ret < 0) ? 1 : 0;

	if (verbose == 0)
		;
	else if (errs)
		printf("FAIL <---  (ret = %d)\n", ret);
	else
		printf("PASS\n");

	return(errs);
}



