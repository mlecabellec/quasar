// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/wait/wait_gsc.c $
// $Rev: 54969 $
// $Date: 2024-08-07 15:58:15 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
int wait_gsc_test(int fd)
{
	int	errs	= 0;

	gsc_label("Wait GSC IRQs");
	printf("\n");
	gsc_label_level_inc();

	errs	+= wait_irq0_initialize_test(fd);
	errs	+= wait_irq0_autocal_test(fd);
	errs	+= wait_irq0_sync_start_test(fd);
	errs	+= wait_irq0_sync_done_test(fd);
	errs	+= wait_irq0_burst_start_test(fd);
	errs	+= wait_irq0_burst_done_test(fd);

	errs	+= wait_irq1_in_buf_thr_l2h_test(fd);
	errs	+= wait_irq1_in_buf_thr_h2l_test(fd);
	errs	+= wait_irq1_in_buf_error_test(fd);

	gsc_label_level_dec();
	return(errs);
}


