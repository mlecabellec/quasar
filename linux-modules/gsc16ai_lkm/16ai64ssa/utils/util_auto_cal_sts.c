// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_auto_cal_sts.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



// This source is retained for backwards compatibility only.

/******************************************************************************
*
*	Function:	ai64ssa_auto_cal_sts
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_AUTO_CAL_STS service.
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

int ai64ssa_auto_cal_sts(int fd, int index, int verbose, s32* get)
{
	char		buf[128];
	int			errs;
	const char*	ptr		= buf;
	int			ret;
	s32			set;

	if (verbose)
		gsc_label_index("Autocal Status", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_AUTO_CAL_STS, &set);
	errs	= ret ? 1 : 0;

	switch (set)
	{
		default:

			errs++;
			sprintf(buf, "Unrecognized option: 0x%lX", (long) set);
			break;

		case AI64SSA_AUTO_CAL_STS_ACTIVE:

			ptr	= "Active";
			break;

		case AI64SSA_AUTO_CAL_STS_FAIL:

			ptr	= "Failed";
			break;

		case AI64SSA_AUTO_CAL_STS_PASS:

			ptr	= "Passed";
			break;
	}

	if (verbose)
		printf("%s  (%s)\n", errs ? "FAIL <---" : "PASS", ptr);

	if (get)
		get[0]	= set;

	return(errs);
}


