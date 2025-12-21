// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/led.c $
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
			/* cmd		*/	OPTO16X16_IOCTL_LED,
			/* arg		*/	OPTO16X16_LED_OFF,
			/* reg		*/	OPTO16X16_GSC_BCSR,
			/* mask		*/	0x80,
			/* value	*/	0x80
		},
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_LED,
			/* arg		*/	OPTO16X16_LED_ON,
			/* reg		*/	OPTO16X16_GSC_BCSR,
			/* mask		*/	0x80,
			/* value	*/	0x00
		},

		{ SERVICE_END_LIST }
	};

	int errs	= 0;

	errs	+= opto16x16_initialize(fd, -1, 0);

	errs	+= service_ioctl_set_reg_list(fd, list);
	errs	+= service_ioctl_reg_get_list(fd, list);

	errs	+= service_ioctl_set_reg_list(fd, list);
	errs	+= service_ioctl_reg_get_list(fd, list);

	errs	+= opto16x16_initialize(fd, -1, 0);
	return(errs);
}



//*****************************************************************************
static int _function_test(int fd)
{
	// The LED requires visual examination to see it go on and off.
	return(0);
}



/******************************************************************************
*
*	Function:	led_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_LED.
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

int led_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_LED");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


