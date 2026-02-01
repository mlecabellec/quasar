// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_aux_in_pol.c $
// $Rev: 42829 $
// $Date: 2018-05-17 17:02:12 -0500 (Thu, 17 May 2018) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_aux_in_pol
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_AUX_IN_POL service.
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

int	ai64ssa_aux_in_pol(int fd, int index, int verbose, s32 set, s32* get)
{
	char	buf[128];
	int		errs;
	int		ret;

	if (verbose)
		gsc_label_index("Aux Input Polarity", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_AUX_IN_POL, &set);
	errs	= ret ? 1 : 0;

	switch (set)
	{
		default:

			sprintf(buf, "INVALID: %ld", (long) set);
			break;

		case AI64SSA_AUX_IN_POL_LO_2_HI:

			strcpy(buf, "Low-to-High");
			break;

		case AI64SSA_AUX_IN_POL_HI_2_LO:

			strcpy(buf, "High-to-Low");
			break;
	}

	if (verbose)
		printf("%s  (%s)\n", errs ? "FAIL <---" : "PASS", buf);

	if (get)
		get[0]	= set;

	return(errs);
}


