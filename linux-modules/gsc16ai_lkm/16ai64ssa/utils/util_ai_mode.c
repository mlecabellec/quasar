// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_ai_mode.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_ai_mode
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_AI_MODE service.
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
*		set		This is the value to apply.
*
*		get		The results are reported here. This may be NULL.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int	ai64ssa_ai_mode(int fd, int index, int verbose, s32 set, s32* get)
{
	char		buf[128];
	int			errs;
	const char*	ptr		= buf;
	int			ret;

	if (verbose)
		gsc_label_index("Analog Input Mode", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_AI_MODE, &set);
	errs	= ret ? 1 : 0;

	switch (set)
	{
		default:

			errs++;
			sprintf(buf, "Unrecognized option: 0x%lX", (long) set);
			break;

		case AI64SSA_AI_MODE_DIFF:

			ptr	= "Differential";
			break;

		case AI64SSA_AI_MODE_SINGLE:

			ptr	= "Single Ended";
			break;

		case AI64SSA_AI_MODE_PS_DIFF:

			ptr	= "Pseudo Differential";
			break;

		case AI64SSA_AI_MODE_ZERO:

			ptr	= "Zero Test";
			break;

		case AI64SSA_AI_MODE_VREF:

			ptr	= "Vref Test";
			break;
	}

	if (verbose)
		printf("%s  (%s)\n", errs ? "FAIL <---" : "PASS", ptr);

	if (get)
		get[0]	= set;

	return(errs);
}


