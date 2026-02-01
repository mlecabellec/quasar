// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/initialize.c $
// $Rev: 51414 $
// $Date: 2022-07-14 14:34:50 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _service_test(int fd)
{
	// There is no initializa bit to exercise.
	return(0);
}



//*****************************************************************************
static int _function_test(int fd)
{
	static const service_data_t	list[]	=
	{
		{				//	Adjust the input buffer threshold.
			/* service	*/	SERVICE_REG_MOD,
			/* cmd		*/	0,
			/* arg		*/	0,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	0xAAAAAA
		},
		{				//	Make sure the value is correct.
			/* service	*/	SERVICE_REG_TEST,
			/* cmd		*/	0,
			/* arg		*/	0,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	0xAAAAAA
		},
		{				//	Perform the initialization.
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_INITIALIZE,
			/* arg		*/	0,
			/* reg		*/	OPTO16X16_GSC_CDR,
			/* mask		*/	0xFFFFFF,
			/* value	*/	0x000000
		},

		{ SERVICE_END_LIST }
	};

	int errs	= 0;

	errs	+= opto16x16_initialize(fd, -1, 0);
	errs	+= service_ioctl_set_reg_list(fd, list);
	errs	+= opto16x16_initialize(fd, -1, 0);

	return(errs);
}



/******************************************************************************
*
*	Function:	initialize_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_INITIALIZE.
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

int initialize_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_INITIALIZE");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


