// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/wait/wait_main.c $
// $Rev: 54969 $
// $Date: 2024-08-07 15:58:15 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _dma(int fd)
{
	char	buf[1024];
	s32		enable_a;
	s32		enable_b;
	int		errs	= 0;
	s32		fsamp	= 20000;
	s32		nrate_a;
	s32		nrate_b;
	s32		qty		= 64;
	int		ret;
	s32		src;
	s32		src_b;

	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_CHANNEL_QTY, &qty);

	errs	+= ai64ssa_initialize		(fd, -1, 0);
	errs	+= ai64ssa_rx_io_mode		(fd, -1, 0, GSC_IO_MODE_BMDMA,				NULL);
	errs	+= ai64ssa_rx_io_overflow	(fd, -1, 0, AI64SSA_IO_OVERFLOW_IGNORE,		NULL);
	errs	+= ai64ssa_rx_io_timeout	(fd, -1, 0, AI64SSA_IO_TIMEOUT_DEFAULT,		NULL);
	errs	+= ai64ssa_rx_io_underflow	(fd, -1, 0, AI64SSA_IO_UNDERFLOW_IGNORE,	NULL);
	errs	+= ai64ssa_ai_range			(fd, -1, 0, AI64SSA_AI_RANGE_10V,			NULL);
	errs	+= ai64ssa_ai_buf_thr_lvl	(fd, -1, 0, 250000,							NULL);
	errs	+= ai64ssa_ai_mode			(fd, -1, 0, AI64SSA_AI_MODE_SINGLE,			NULL);
	errs	+= ai64ssa_burst_size		(fd, -1, 0, 1,								NULL);
	errs	+= ai64ssa_burst_src		(fd, -1, 0, AI64SSA_BURST_SRC_DISABLE,		NULL);
	errs	+= ai64ssa_data_packing		(fd, -1, 0, AI64SSA_DATA_PACKING_DISABLE,	NULL);
	errs	+= ai64ssa_fsamp_ai_compute	(fd, -1, 0, fsamp, &src, &src_b, &nrate_a, &nrate_b, &enable_a, &enable_b, NULL);
	errs	+= ai64ssa_samp_clk_src		(fd, -1, 0, src,							NULL);
	errs	+= ai64ssa_rbg_clk_src		(fd, -1, 0, src_b,							NULL);
	errs	+= ai64ssa_rag_nrate		(fd, -1, 0, nrate_a,						NULL);
	errs	+= ai64ssa_rbg_nrate		(fd, -1, 0, nrate_b,						NULL);
	errs	+= ai64ssa_rag_enable		(fd, -1, 0, enable_a,						NULL);
	errs	+= ai64ssa_rbg_enable		(fd, -1, 0, enable_b,						NULL);
	errs	+= ai64ssa_scan_marker		(fd, -1, 0, AI64SSA_SCAN_MARKER_DISABLE,	NULL);

	ret	= ai64ssa_read(fd, buf, sizeof(buf));

	if (ret < 0)
	{
		errs++;
		printf("ERROR: ai64ssa_read() failure, returned %d\n", ret);
	}

	return(errs);
}




//*****************************************************************************
static int _irq_main_dma_test(int fd)
{
	int	errs	= 0;

	gsc_label("DMA0/DMA1");
	errs	+= wait_test(fd, GSC_WAIT_MAIN_DMA0 | GSC_WAIT_MAIN_DMA1, 0, 0, _dma);
	return(errs);
}



//*****************************************************************************
static int _gsc(int fd)
{
	int	errs;

	errs	= ai64ssa_initialize(fd, -1, 0);
	return(errs);
}



//*****************************************************************************
static int _irq_main_gsc_test(int fd)
{
	int	errs	= 0;

	gsc_label("GSC");
	errs	+= wait_test(fd, GSC_WAIT_MAIN_GSC, 0, 0, _gsc);
	return(errs);
}



//*****************************************************************************
static int _pci(int fd)
{
	int	errs;

	errs	= ai64ssa_initialize(fd, -1, 0);
	return(errs);
}



//*****************************************************************************
static int _irq_main_pci_test(int fd)
{
	int	errs	= 0;

	gsc_label("PCI");
	errs	+= wait_test(fd, GSC_WAIT_MAIN_PCI, 0, 0, _pci);
	return(errs);
}



//*****************************************************************************
int wait_main_test(int fd)
{
	int	errs	= 0;

	gsc_label("Wait Main IRQs");
	printf("\n");
	gsc_label_level_inc();

	errs	+= _irq_main_dma_test(fd);
	errs	+= _irq_main_gsc_test(fd);
	errs	+= _irq_main_pci_test(fd);

	gsc_label_level_dec();
	return(errs);
}


