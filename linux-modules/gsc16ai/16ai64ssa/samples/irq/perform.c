// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/irq/perform.c $
// $Rev: 53544 $
// $Date: 2023-08-07 14:21:13 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int	errs	= 0;

	errs	+= irq_main_test(args->fd);
	errs	+= irq_0_test(args->fd);
	errs	+= irq_1_test(args->fd);

	return(errs);
}


