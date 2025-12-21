// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/wait/wait_irq1.c $
// $Rev: 51344 $
// $Date: 2022-07-11 17:35:22 -0500 (Mon, 11 Jul 2022) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _low_2_high(int fd)
{
	int	errs	= 0;

	errs	+= ai64ssa_initialize		(fd, -1, 0);
	errs	+= ai64ssa_ai_buf_thr_lvl	(fd, -1, 0, 16, NULL);
	errs	+= ai64ssa_irq1_sel			(fd, -1, 0, AI64SSA_IRQ1_IN_BUF_THR_L2H, NULL);
	errs	+= ai64ssa_rag_enable		(fd, -1, 0, AI64SSA_GEN_ENABLE_YES, NULL);
	gsc_time_sleep_ms(1000);
	return(errs);
}



//*****************************************************************************
int wait_irq1_in_buf_thr_l2h_test(int fd)
{
	int	errs;

	gsc_label("Buffer Threshold Low-to-High");
	errs	= wait_test(fd, 0, AI64SSA_WAIT_GSC_IN_BUF_THR_L2H, 0, _low_2_high);
	return(errs);
}



//*****************************************************************************
static int _high_2_low(int fd)
{
	int	errs	= 0;

	errs	+= ai64ssa_initialize		(fd, -1, 0);
	errs	+= ai64ssa_ai_buf_thr_lvl	(fd, -1, 0, 16, NULL);
	errs	+= ai64ssa_irq1_sel			(fd, -1, 0, AI64SSA_IRQ1_IN_BUF_THR_H2L, NULL);
	errs	+= ai64ssa_rag_enable		(fd, -1, 0, AI64SSA_GEN_ENABLE_YES, NULL);
	gsc_time_sleep_ms(1000);
	errs	+= ai64ssa_rag_enable		(fd, -1, 0, AI64SSA_GEN_ENABLE_NO, NULL);
	errs	+= ai64ssa_ai_buf_clear		(fd, -1, 0);
	return(errs);
}



//*****************************************************************************
int wait_irq1_in_buf_thr_h2l_test(int fd)
{
	int	errs;

	gsc_label("Buffer Threshold High-to-Low");
	errs	= wait_test(fd, 0, AI64SSA_WAIT_GSC_IN_BUF_THR_H2L, 0, _high_2_low);
	return(errs);
}



//*****************************************************************************
static int _error_overflow(int fd)
{
	int	errs	= 0;

	errs	+= ai64ssa_initialize	(fd, -1, 0);
	errs	+= ai64ssa_irq1_sel		(fd, -1, 0, AI64SSA_IRQ1_BUF_ERROR, NULL);
	errs	+= ai64ssa_rag_enable	(fd, -1, 0, AI64SSA_GEN_ENABLE_YES, NULL);
	gsc_time_sleep_ms(1000);
	return(errs);
}



//*****************************************************************************
static int _error_underflow(int fd)
{
	int	errs	= 0;
	u32	idbr;

	errs	+= ai64ssa_initialize	(fd, -1, 0);
	errs	+= ai64ssa_irq1_sel		(fd, -1, 0, AI64SSA_IRQ1_BUF_ERROR, NULL);
	errs	+= ai64ssa_rag_enable	(fd, -1, 0, AI64SSA_GEN_ENABLE_YES, NULL);
	errs	+= ai64ssa_reg_read		(fd, -1, 0, AI64SSA_GSC_IDBR, &idbr);
	return(errs);
}



//*****************************************************************************
int wait_irq1_in_buf_error_test(int fd)
{
	s32	buf_err	= 0;
	int	errs;

	gsc_label("Buffer Error");

	errs	= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_IRQ1_BUF_ERROR, &buf_err);

	if (buf_err == 0)
	{
		printf("SKIPPED  (Not supported on this device.\n");
	}
	else
	{
		printf("\n");
		gsc_label_level_inc();

		gsc_label("Overflow");
		errs	+= wait_test(fd, 0, AI64SSA_WAIT_GSC_BUF_ERROR, 0, _error_overflow);

		gsc_label("Underflow");
		errs	+= wait_test(fd, 0, AI64SSA_WAIT_GSC_BUF_ERROR, 0, _error_underflow);

		gsc_label_level_dec();
	}

	return(errs);
}


