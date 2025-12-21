// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_count.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_count_boards
*
*	Purpose:
*
*		Count the number of installed boards.
*
*	Arguments:
*
*		verbose	Work verbosely?
*
*		get		The results are reported here. This may be NULL.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int ai64ssa_count_boards(int verbose, s32* get)
{
	int	errs;

	errs	= os_count_boards(verbose, get, ai64ssa_open, ai64ssa_read, ai64ssa_close);
	return(errs);
}



