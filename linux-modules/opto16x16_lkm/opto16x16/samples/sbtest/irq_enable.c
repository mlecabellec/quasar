// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/irq_enable.c $
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
			/* cmd		*/	OPTO16X16_IOCTL_IRQ_ENABLE,
			/* arg		*/	0,
			/* reg		*/	OPTO16X16_GSC_CIER,
			/* mask		*/	0x1FFFF,
			/* value	*/	0x00000
		},
		{
			/* service	*/	SERVICE_REG_TEST,
			/* cmd		*/	0,
			/* arg		*/	0,
			/* reg		*/	OPTO16X16_GSC_BCSR,
			/* mask		*/	0x40,
			/* value	*/	0x00
		},

		{ SERVICE_END_LIST }
	};

	int errs	= 0;
	int	i;

	errs	+= opto16x16_initialize(fd, -1, 0);

	for (i = 0; i <= 15; i++)
	{
		// Walking one.
		list[0].arg		= 0xFFFF & (1 << i);
		list[0].value	= list[0].arg;
		list[1].value	= (i == 16) ? 0x40 : 0x00;

		errs	+= service_ioctl_set_reg_list(fd, list);
		errs	+= service_ioctl_reg_get_list(fd, list);

		errs	+= service_ioctl_set_reg_list(fd, list);
		errs	+= service_ioctl_reg_get_list(fd, list);

		// Walking zero.
		list[0].arg		= 0x1FFFF ^ (1 << i);
		list[0].value	= 0x0FFFF & list[0].arg;
		list[1].value	= (i == 16) ? 0x00 : 0x40;

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
	// This requires additional equipment.
	return(0);
}



/******************************************************************************
*
*	Function:	irq_enable_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_IRQ_ENABLE.
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

int irq_enable_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_IRQ_ENABLE");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


