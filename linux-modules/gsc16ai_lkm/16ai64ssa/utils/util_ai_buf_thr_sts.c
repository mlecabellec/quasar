// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_ai_buf_thr_sts.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_ai_buf_thr_sts
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_AI_BUF_THR_STS service.
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

int ai64ssa_ai_buf_thr_sts(int fd, int index, int verbose, s32* get)
{
	char		buf[128];
	int			errs;
	const char*	ptr		= buf;
	int			ret;
	s32			set;

	if (verbose)
		gsc_label_index("Input Buffer Thresh. Status", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_AI_BUF_THR_STS, &set);
	errs	= ret ? 1 : 0;

	switch (set)
	{
		default:

			errs++;
			sprintf(buf, "INVALID: %ld", (long) set);
			break;

		case AI64SSA_AI_BUF_THR_STS_CLEAR:

			ptr	= "Clear";
			break;

		case AI64SSA_AI_BUF_THR_STS_SET:

			ptr	= "Set";
			break;
	}

	if (verbose)
		printf("%s  (%s)\n", errs ? "FAIL <---" : "PASS", ptr);

	if (get)
		get[0]	= set;

	return(errs);
}


