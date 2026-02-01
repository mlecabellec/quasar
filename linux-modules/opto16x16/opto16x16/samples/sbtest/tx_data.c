// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/tx_data.c $
// $Rev: 51414 $
// $Date: 2022-07-14 14:34:50 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _service_test(int fd)
{
	service_data_t	list[]	=
	{
		{
			/* service	*/	SERVICE_NORMAL,
			/* cmd		*/	OPTO16X16_IOCTL_TX_DATA,
			/* arg		*/	0,
			/* reg		*/	OPTO16X16_GSC_ODR,
			/* mask		*/	0xFFFF,
			/* value	*/	0x0000
		},

		{ SERVICE_END_LIST }
	};

	int errs	= 0;
	int	i;

	errs	+= opto16x16_initialize(fd, -1, 0);

	for (i = 0; i <= 15; i++)
	{
		// Walking one.
		list[0].arg		= 1 << i;
		list[0].value	= list[0].arg;

		errs	+= service_ioctl_set_reg_list(fd, list);
		errs	+= service_ioctl_reg_get_list(fd, list);

		errs	+= service_ioctl_set_reg_list(fd, list);
		errs	+= service_ioctl_reg_get_list(fd, list);

		// Walking zero.
		list[0].arg		= 0xFFFF ^ (1 << i);
		list[0].value	= list[0].arg;

		errs	+= service_ioctl_set_reg_list(fd, list);
		errs	+= service_ioctl_reg_get_list(fd, list);

		errs	+= service_ioctl_set_reg_list(fd, list);
		errs	+= service_ioctl_reg_get_list(fd, list);
	}

	errs	+= opto16x16_initialize(fd, -1, 0);
	return(errs);
}



//*****************************************************************************
static int _function_test(int fd)
{
	// Additional equipment is required to verify this.
	return(0);
}



/******************************************************************************
*
*	Function:	tx_data_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_TX_DATA.
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

int tx_data_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_TX_DATA");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


