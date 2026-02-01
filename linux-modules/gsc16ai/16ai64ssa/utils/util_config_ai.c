// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_config_ai.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_config_ai
*
*	Purpose:
*
*		Configure the given device's analog input options.
*
*	Arguments:
*
*		fd		The handle to use to access the driver.
*
*		index	The index of the device to access. Ignore if < 0.
*
*		verbose	Work verbosely?
*
*		fsamp	This is the desired Fsamp rate, or -1 to use the default.
*
*	Returned:
*
*		>= 0	The number of errors encountered here.
*
******************************************************************************/

int	ai64ssa_config_ai(int fd, int index, int verbose, s32 fsamp)
{
	s32	chans;
	s32	enable_a;
	s32	enable_b;
	int	errs		= 0;
	s32	nrate_a;
	s32	nrate_b;
	s32	qty			= 32;
	s32	range;
	s32	src;
	s32	src_b;

	if (verbose)
	{
		gsc_label_index("Input Configuration", index);
		printf("\n");
		gsc_label_level_inc();
	}

	errs	+= ai64ssa_initialize			(fd, index, verbose);
	errs	+= ai64ssa_query				(fd, index, verbose, AI64SSA_QUERY_CHANNEL_QTY, &qty);

	// Settings must be applied after the initialization call!

	errs	+= ai64ssa_rx_io_mode			(fd, index, verbose, GSC_IO_MODE_PIO,				NULL);
	errs	+= ai64ssa_rx_io_overflow		(fd, index, verbose, AI64SSA_IO_OVERFLOW_CHECK,		NULL);
	errs	+= ai64ssa_rx_io_timeout		(fd, index, verbose, AI64SSA_IO_TIMEOUT_DEFAULT,	NULL);
	errs	+= ai64ssa_rx_io_underflow		(fd, index, verbose, AI64SSA_IO_UNDERFLOW_CHECK,	NULL);
	errs	+= ai64ssa_ai_range				(fd, index, verbose, AI64SSA_AI_RANGE_10V,			NULL);
	errs	+= ai64ssa_ai_buf_thr_lvl		(fd, index, verbose, 250000,						NULL);
	errs	+= ai64ssa_ai_mode				(fd, index, verbose, AI64SSA_AI_MODE_SINGLE,		NULL);
	errs	+= ai64ssa_burst_size			(fd, index, verbose, 1,								NULL);
	errs	+= ai64ssa_burst_src			(fd, index, verbose, AI64SSA_BURST_SRC_DISABLE,		NULL);

	errs	+= ai64ssa_query				(fd, index, verbose, AI64SSA_QUERY_CHAN_RANGE, &range);

	if (range)
		chans	= AI64SSA_CHAN_ACTIVE_RANGE;
	else if (qty >= 64)
		chans	= AI64SSA_CHAN_ACTIVE_0_63;
	else
		chans	= AI64SSA_CHAN_ACTIVE_0_31;

	errs	+= ai64ssa_chan_active			(fd, index, verbose, chans,							NULL);

	errs	+= ai64ssa_chan_first			(fd, index, verbose, 0,								NULL);

	errs	+= ai64ssa_chan_last			(fd, index, verbose, qty - 1,						NULL);

	errs	+= ai64ssa_chan_single			(fd, index, verbose, 0,								NULL);
	errs	+= ai64ssa_data_format			(fd, index, verbose, AI64SSA_DATA_FORMAT_OFF_BIN,	NULL);
	errs	+= ai64ssa_data_packing			(fd, index, verbose, AI64SSA_DATA_PACKING_DISABLE,	NULL);
	errs	+= ai64ssa_fsamp_ai_compute		(fd, index, verbose, fsamp, &src, &src_b, &nrate_a, &nrate_b, &enable_a, &enable_b, NULL);
	errs	+= ai64ssa_samp_clk_src			(fd, index, verbose, src,							NULL);
	errs	+= ai64ssa_rbg_clk_src			(fd, index, verbose, src_b,							NULL);
	errs	+= ai64ssa_rag_nrate			(fd, index, verbose, nrate_a,						NULL);
	errs	+= ai64ssa_rbg_nrate			(fd, index, verbose, nrate_b,						NULL);
	errs	+= ai64ssa_rag_enable			(fd, index, verbose, enable_a,						NULL);
	errs	+= ai64ssa_rbg_enable			(fd, index, verbose, enable_b,						NULL);
	errs	+= ai64ssa_scan_marker			(fd, index, verbose, AI64SSA_SCAN_MARKER_DISABLE,	NULL);
	errs	+= ai64ssa_scan_marker_set		(fd, index, verbose, 0);

	errs	+= ai64ssa_fsamp_ai_report_all	(fd, index, verbose,								NULL);

	// Settings affecting the sample rate or voltage range must generally
	// be applied before autocalibration.
	// Refer to the board user manual for clarification.
	errs	+= ai64ssa_autocal				(fd, index, verbose);

	// Interrupt selections should be applied after autocalibration as the
	// driver overwrites the current selection to detect the Autocalibration
	// Done interrupt. If an application intends to detect the Autocalibration
	// Done interrupt, then the interrupt selection may be made beforehand.
	errs	+= ai64ssa_irq0_sel				(fd, index, verbose, AI64SSA_IRQ0_INIT_DONE,		NULL);
	errs	+= ai64ssa_irq1_sel				(fd, index, verbose, AI64SSA_IRQ1_NONE,				NULL);

	// The final step is to clear the buffer. This service clears the FIFO and
	// resets the overflow and underflow status bits.
	errs	+= ai64ssa_ai_buf_clear			(fd, index, verbose);

	if (verbose)
		gsc_label_level_dec();

	return(errs);
}


