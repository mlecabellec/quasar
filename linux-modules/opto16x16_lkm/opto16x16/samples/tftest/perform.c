// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/tftest/perform.c $
// $Rev: 53725 $
// $Date: 2023-09-14 10:41:18 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int	errs	= 0;

	errs	+= d00_test(args->fd);
	errs	+= d01_test(args->fd);
	errs	+= d02_test(args->fd);
	errs	+= d03_test(args->fd);
	errs	+= d04_test(args->fd);
	errs	+= d05_test(args->fd);
	errs	+= d06_test(args->fd);
	errs	+= d07_test(args->fd);
	errs	+= d08_test(args->fd);
	errs	+= d09_test(args->fd);
	errs	+= d10_test(args->fd);
	errs	+= d11_test(args->fd);
	errs	+= d12_test(args->fd);
	errs	+= d13_test(args->fd);
	errs	+= d14_test(args->fd);
	errs	+= d15_test(args->fd);
	errs	+= rx_event_counter_test(args->fd);

	return(errs);
}


