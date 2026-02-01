// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_rx_data.c $
// $Rev: 51397 $
// $Date: 2022-07-14 13:14:08 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_rx_data
*
*	Purpose:
*
*		Provide a visual wrapper for the OPTO16X16_IOCTL_RX_DATA service.
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

int opto16x16_rx_data(int fd, int index, int verbose, s32* get)
{
	int	errs;
	int	ret;
	s32	set;

	if (verbose)
		gsc_label_index("Rx Data", index);

	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_RX_DATA, &set);
	errs	= ret ? 1 : 0;

	if (verbose)
		printf("%s  (0x%04lX)\n", errs ? "FAIL <---" : "PASS", (long) set);

	if (get)
		get[0]	= set;

	return(errs);
}


