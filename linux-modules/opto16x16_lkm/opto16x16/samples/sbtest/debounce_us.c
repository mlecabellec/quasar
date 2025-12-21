// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/debounce_us.c $
// $Rev: 51414 $
// $Date: 2022-07-14 14:34:50 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _service_test(int fd)
{
	static const service_data_t	list[]	=
	{
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_US,
			/* arg		*/	0,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	0
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_US,
			/* arg		*/	1,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	2
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_US,
			/* arg		*/	12,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	39
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_US,
			/* arg		*/	123,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	409
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_US,
			/* arg		*/	1234,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	4112
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_US,
			/* arg		*/	12345,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	41149L
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_US,
			/* arg		*/	123456L,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	411519L
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_US,
			/* arg		*/	200000L,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	666665L
		},

		{ SERVICE_END_LIST }
	};

	int errs	= 0;

	errs	+= opto16x16_initialize(fd, -1, 0);
	errs	+= service_ioctl_set_get_list(fd, list);
	errs	+= service_ioctl_set_get_list(fd, list);
	errs	+= opto16x16_initialize(fd, -1, 0);

	return(errs);
}



//*****************************************************************************
static int _function_test(int fd)
{
	// Additional equipment is required to test this.
	return(0);
}



/******************************************************************************
*
*	Function:	debounce_us_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_DEBOUNCE_US.
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

int debounce_us_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_DEBOUNCE_US");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


