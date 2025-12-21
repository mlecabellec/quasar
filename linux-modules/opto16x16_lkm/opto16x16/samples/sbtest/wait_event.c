// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/wait_event.c $
// $Rev: 51414 $
// $Date: 2022-07-14 14:34:50 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Sample Application: source file

#include "main.h"



/******************************************************************************
*
*	Function:	wait_event_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_WAIT_EVENT.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int wait_event_test(int fd)
{
	gsc_label("OPTO16X16_IOCTL_WAIT_EVENT");

	// The only wait events supported by this device are for interrupts.
	// We can't test any of the interrupts without external inputs
	// so no testing is performed here.
	printf("SKIPPED\n");
	return(0);
}


