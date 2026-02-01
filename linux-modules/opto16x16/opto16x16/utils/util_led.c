// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_led.c $
// $Rev: 44253 $
// $Date: 2018-12-10 12:58:41 -0600 (Mon, 10 Dec 2018) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_led
*
*	Purpose:
*
*		Provide a visual wrapper for the OPTO16X16_IOCTL_LED service.
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

int opto16x16_led(int fd, int index, int verbose, s32 set, s32* get)
{
	char		buf[128];
	int			errs;
	const char*	ptr;
	int			ret;

	if (verbose)
		gsc_label_index("LED", index);

	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_LED, &set);
	errs	= ret ? 1 : 0;

	switch (set)
	{
		default:

			errs	= 1;
			ptr		= buf;
			sprintf(buf, "Unrecognized option: 0x%lX", (long) set);
			break;

		case OPTO16X16_LED_OFF:

			ptr	= "Off";
			break;

		case OPTO16X16_LED_ON:

			ptr	= "On";
			break;
	}

	if (verbose)
		printf("%s  (%s)\n", errs ? "FAIL <---" : "PASS", ptr);

	if (get)
		get[0]	= set;

	return(errs);
}


