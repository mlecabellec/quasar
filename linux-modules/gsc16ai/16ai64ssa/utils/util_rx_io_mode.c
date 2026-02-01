// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_rx_io_mode.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_rx_io_mode
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_RX_IO_MODE service.
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

int	ai64ssa_rx_io_mode(int fd, int index, int verbose, s32 set, s32* get)
{
	char		buf[128];
	int			errs;
	const char*	ptr		= buf;
	int			ret;

	if (verbose)
		gsc_label_index("Rx I/O Mode", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RX_IO_MODE, &set);
	errs	= ret ? 1 : 0;

	switch (set)
	{
		default:

			errs++;
			sprintf(buf, "Unrecognized option: 0x%lX", (long) set);
			break;

		case GSC_IO_MODE_PIO:

			ptr	= "PIO";
			break;

		case GSC_IO_MODE_BMDMA:

			ptr	= "Block Mode DMA";
			break;

		case GSC_IO_MODE_DMDMA:

			ptr	= "Demand Mode DMA";
			break;
	}

	if (verbose)
		printf("%s  (%s)\n", errs ? "FAIL <---" : "PASS", ptr);

	if (get)
		get[0]	= set;

	return(errs);
}


