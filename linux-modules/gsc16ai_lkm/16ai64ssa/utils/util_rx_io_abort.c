// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_rx_io_abort.c $
// $Rev: 42829 $
// $Date: 2018-05-17 17:02:12 -0500 (Thu, 17 May 2018) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_rx_io_abort
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_RX_IO_ABORT service.
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

int ai64ssa_rx_io_abort(int fd, int index, int verbose, s32* get)
{
	char		buf[128];
	int			errs;
	const char*	ptr;
	int			ret;
	s32			set;

	if (verbose)
		gsc_label_index("Rx I/O Abort", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RX_IO_ABORT, &set);
	errs	= ret ? 1 : 0;

	switch (set)
	{
		default:

			errs++;
			ptr	= buf;
			sprintf(buf, "Unrecognized option: 0x%lX", (long) set);
			break;

		case AI64SSA_IO_ABORT_NO:

			ptr	= "No, Not Aborted";
			break;

		case AI64SSA_IO_ABORT_YES:

			ptr	= "Yes, Aborted";
			break;
	}

	if (verbose)
		printf("%s  (%s)\n", errs ? "FAIL <---" : "PASS", ptr);

	if (get)
		get[0]	= set;

	return(errs);
}


