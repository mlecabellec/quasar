// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_irq0_sel.c $
// $Rev: 53557 $
// $Date: 2023-08-07 14:30:23 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_irq0_sel
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_IRQ0_SEL service.
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

int	ai64ssa_irq0_sel(int fd, int index, int verbose, s32 set, s32* get)
{
	char	buf[128];
	int		errs;
	int		ret;

	if (verbose)
		gsc_label_index("IRQ0 Selection", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_IRQ0_SEL, &set);
	errs	= ret ? 1 : 0;

	switch (set)
	{
		default:

			sprintf(buf, "INVALID: %ld", (long) set);
			break;

		case AI64SSA_IRQ0_INIT_DONE:

			strcpy(buf, "Initialization Done");
			break;

		case AI64SSA_IRQ0_AUTOCAL_DONE:

			strcpy(buf, "Autocalibration Done");
			break;

		case AI64SSA_IRQ0_SYNC_START:

			strcpy(buf, "Sync Start");
			break;

		case AI64SSA_IRQ0_SYNC_DONE:

			strcpy(buf, "Sync Done");
			break;

		case AI64SSA_IRQ0_BURST_START:

			strcpy(buf, "Burst Start");
			break;

		case AI64SSA_IRQ0_BURST_DONE:

			strcpy(buf, "Burst Done");
			break;
	}

	if (verbose)
		printf("%s  (%s)\n", errs ? "FAIL <---" : "PASS", buf);

	if (get)
		get[0]	= set;

	return(errs);
}


