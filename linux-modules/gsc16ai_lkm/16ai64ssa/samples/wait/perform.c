// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/wait/perform.c $
// $Rev: 53553 $
// $Date: 2023-08-07 14:25:13 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int	errs	= 0;

	errs	+= wait_main_test(args->fd);
	errs	+= wait_gsc_test(args->fd);
	errs	+= wait_io_test(args->fd);

	return(errs);
}


