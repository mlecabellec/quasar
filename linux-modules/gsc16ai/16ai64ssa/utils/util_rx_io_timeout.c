// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_rx_io_timeout.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_rx_io_timeout
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_RX_IO_TIMEOUT service.
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

int	ai64ssa_rx_io_timeout(int fd, int index, int verbose, s32 set, s32* get)
{
	char		buf[128];
	int			errs;
	const char*	ptr;
	int			ret;

	if (verbose)
		gsc_label_index("Rx I/O Timeout", index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RX_IO_TIMEOUT, &set);
	errs	= ret ? 1 : 0;

	if (verbose)
	{
		if (set == -1)
		{
			ptr	= "This service is not supported by this device.";
		}
		else if (set == AI64SSA_IO_TIMEOUT_INFINITE)
		{
			ptr	= "infinite/never timeout";
		}
		else
		{
			sprintf(buf, "%ld second%s", (long) set, (set == 1) ? "" : "s");
			ptr	= buf;
		}

		printf("%s  (%s)\n", errs ? "FAIL <---" : "PASS", ptr);
	}

	if (get)
		get[0]	= set;

	return(errs);
}


