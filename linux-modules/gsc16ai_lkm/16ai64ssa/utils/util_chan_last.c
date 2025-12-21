// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_chan_last.c $
// $Rev: 42829 $
// $Date: 2018-05-17 17:02:12 -0500 (Thu, 17 May 2018) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_chan_last
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_CHAN_LAST service.
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
*		set		The setting to apply.
*
*		get		The current setting is recorded here, if not NULL.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int ai64ssa_chan_last(int fd, int index, int verbose, s32 set, s32* get)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Last Channel Selection", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_CHAN_LAST, &set);
	errs	= ret ? 1 : 0;

	if (verbose)
	{
		printf("%s  (", errs ? "FAIL <---" : "PASS");
		gsc_label_long_comma(set);
		printf(")\n");
	}

	if (get)
		get[0]	= set;

	return(errs);
}


