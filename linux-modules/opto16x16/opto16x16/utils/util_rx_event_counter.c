// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_rx_event_counter.c $
// $Rev: 48471 $
// $Date: 2020-11-13 14:42:46 -0600 (Fri, 13 Nov 2020) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_rx_event_counter
*
*	Purpose:
*
*		Provide a visual wrapper for the OPTO16X16_IOCTL_RX_EVENT_COUNTER service.
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

int opto16x16_rx_event_counter(int fd, int index, int verbose, s32 set, s32* get)
{
	int	errs;
	int	ret;

	if (verbose)
		gsc_label_index("Rx Event Counter", index);

	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_RX_EVENT_COUNTER, &set);
	errs	= ret ? 1 : 0;

	if (verbose)
		printf("%s  (%ld events)\n", errs ? "FAIL <---" : "PASS", (long) set);

	if (get)
		get[0]	= set;

	return(errs);
}


