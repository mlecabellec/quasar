// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_burst_status.c $
// $Rev: 42829 $
// $Date: 2018-05-17 17:02:12 -0500 (Thu, 17 May 2018) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_burst_status
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_BURST_STATUS service.
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
*		get		The current setting is recorded here, if not NULL.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int	ai64ssa_burst_status(int fd, int index, int verbose, s32* get)
{
	char	buf[128];
	int		errs;
	int		ret;
	s32		set;

	if (verbose)
		gsc_label_index("Burst Status", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_BURST_STATUS, &set);
	errs	= ret ? 1 : 0;

	switch (set)
	{
		default:

			sprintf(buf, "INVALID: %ld", (long) set);
			break;

		case AI64SSA_BURST_STATUS_IDLE:

			strcpy(buf, "Idle");
			break;

		case AI64SSA_BURST_STATUS_ACTIVE:

			strcpy(buf, "Active");
			break;
	}

	if (verbose)
		printf("%s  (%s)\n", errs ? "FAIL <---" : "PASS", buf);

	if (get)
		get[0]	= set;

	return(errs);
}


