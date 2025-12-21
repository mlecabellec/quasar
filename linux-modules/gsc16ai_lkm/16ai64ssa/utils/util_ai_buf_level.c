// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_ai_buf_level.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_ai_buf_level
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_AI_BUF_LEVEL service.
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
*		get		The results are reported here. This may be NULL.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int	ai64ssa_ai_buf_level(int fd, int index, int verbose, s32* get)
{
	int	errs;
	int	ret;
	s32	set;

	if (verbose)
		gsc_label_index("Input Buffer Level", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_AI_BUF_LEVEL, &set);
	errs	= ret ? 1 : 0;

	if (verbose)
		printf("%s  (0x%lX)\n", errs ? "FAIL <---" : "PASS", (long) set);

	if (get)
		get[0]	= set;

	return(errs);
}


