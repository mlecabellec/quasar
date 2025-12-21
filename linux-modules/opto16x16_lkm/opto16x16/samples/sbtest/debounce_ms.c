// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/debounce_ms.c $
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
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_MS,
			/* arg		*/	0,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	0
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_MS,
			/* arg		*/	1,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	3332
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_MS,
			/* arg		*/	22,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	73665L
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_MS,
			/* arg		*/	333,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	1109999L
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_DEBOUNCE_MS,
			/* arg		*/	2000,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	6666665L
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
*	Function:	debounce_ms_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_DEBOUNCE_MS.
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

int debounce_ms_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_DEBOUNCE_MS");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


