// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/wait/wait_irq0.c $
// $Rev: 53553 $
// $Date: 2023-08-07 14:25:13 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _init(int fd)
{
	int	errs;

	errs	= ai64ssa_initialize(fd, -1, 0);
	return(errs);
}



//*****************************************************************************
int wait_irq0_initialize_test(int fd)
{
	int	errs;

	gsc_label("Initialize");
	errs	= wait_test(fd, 0, AI64SSA_WAIT_GSC_INIT_DONE, 0, _init);
	return(errs);
}



//*****************************************************************************
static int _autocal(int fd)
{
	int	errs	= 0;

	errs	+= ai64ssa_initialize	(fd, -1, 0);
	errs	+= ai64ssa_autocal		(fd, -1, 0);
	return(errs);
}



//*****************************************************************************
int wait_irq0_autocal_test(int fd)
{
	int	errs;

	gsc_label("Autocalibrate");
	errs	= wait_test(fd, 0, AI64SSA_WAIT_GSC_AUTOCAL_DONE, 0, _autocal);
	return(errs);
}



//*****************************************************************************
static int _sync_start(int fd)
{
	int	errs	= 0;

	errs	+= ai64ssa_initialize	(fd, -1, 0);
	errs	+= ai64ssa_samp_clk_src	(fd, -1, 0, AI64SSA_SAMP_CLK_SRC_BCR,	NULL);
	errs	+= ai64ssa_irq0_sel		(fd, -1, 0, AI64SSA_IRQ0_SYNC_START, NULL);
	errs	+= ai64ssa_input_sync	(fd, -1, 0);
	return(errs);
}



//*****************************************************************************
int wait_irq0_sync_start_test(int fd)
{
	int	errs;

	gsc_label("SYNC Start");
	errs	= wait_test(fd, 0, AI64SSA_WAIT_GSC_SYNC_START, 0, _sync_start);
	return(errs);
}



//*****************************************************************************
static int _sync_done(int fd)
{
	int	errs	= 0;

	errs	+= ai64ssa_initialize	(fd, -1, 0);
	errs	+= ai64ssa_samp_clk_src	(fd, -1, 0, AI64SSA_SAMP_CLK_SRC_BCR,	NULL);
	errs	+= ai64ssa_irq0_sel		(fd, -1, 0, AI64SSA_IRQ0_SYNC_DONE, NULL);
	errs	+= ai64ssa_input_sync	(fd, -1, 0);
	gsc_time_sleep_ms(100);
	return(errs);
}



//*****************************************************************************
int wait_irq0_sync_done_test(int fd)
{
	int	errs;

	gsc_label("SYNC done");
	errs	= wait_test(fd, 0, AI64SSA_WAIT_GSC_SYNC_DONE, 0, _sync_done);
	return(errs);
}



//*****************************************************************************
static int _burst_start(int fd)
{
	int	errs	= 0;

	errs	+= ai64ssa_initialize	(fd, -1, 0);
	errs	+= ai64ssa_burst_size	(fd, -1, 0, 1, NULL);
	errs	+= ai64ssa_burst_src	(fd, -1, 0, AI64SSA_BURST_SRC_BCR, NULL);
	errs	+= ai64ssa_rag_enable	(fd, -1, 0, AI64SSA_GEN_ENABLE_YES, NULL);
	errs	+= ai64ssa_irq0_sel		(fd, -1, 0, AI64SSA_IRQ0_BURST_START, NULL);
	errs	+= ai64ssa_input_sync	(fd, -1, 0);
	return(errs);
}



//*****************************************************************************
int wait_irq0_burst_start_test(int fd)
{
	int	errs;

	gsc_label("Burst Start");
	errs	= wait_test(fd, 0, AI64SSA_WAIT_GSC_BURST_START, 0, _burst_start);
	return(errs);
}



//*****************************************************************************
static int _burst_done(int fd)
{
	int	errs	= 0;

	errs	+= ai64ssa_initialize	(fd, -1, 0);
	errs	+= ai64ssa_burst_size	(fd, -1, 0, 1, NULL);
	errs	+= ai64ssa_burst_src	(fd, -1, 0, AI64SSA_BURST_SRC_BCR, NULL);
	errs	+= ai64ssa_rag_enable	(fd, -1, 0, AI64SSA_GEN_ENABLE_YES, NULL);
	errs	+= ai64ssa_irq0_sel		(fd, -1, 0, AI64SSA_IRQ0_BURST_DONE, NULL);
	errs	+= ai64ssa_input_sync	(fd, -1, 0);
	gsc_time_sleep_ms(1000);
	return(errs);
}



//*****************************************************************************
int wait_irq0_burst_done_test(int fd)
{
	int	errs;

	gsc_label("Burst Done");
	errs	= wait_test(fd, 0, AI64SSA_WAIT_GSC_BURST_DONE, 0, _burst_done);
	return(errs);
}



