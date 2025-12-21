// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/signals/perform.c $
// $Rev: 54965 $
// $Date: 2024-08-07 15:55:54 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
int perform_tests(const args_t* args)
{
	char	buf[64];
	int		errs	= 0;
	s32		fref;
	s32		nrate;
	s32		sps;

	//===============================================================
	gsc_label("Setup");
	printf("\n");
	gsc_label_level_inc();

	errs	+= ai64ssa_initialize(args->fd, -1, 0);


	gsc_label_level_dec();

	//===============================================================
	gsc_label("SYNC Output");
	printf("\n");
	gsc_label_level_inc();

	errs	+= ai64ssa_query			(args->fd, -1, 0, AI64SSA_QUERY_MASTER_CLOCK, &fref);
	errs	+= ai64ssa_samp_clk_src		(args->fd, -1, 1, AI64SSA_SAMP_CLK_SRC_RAG,	NULL);
	nrate	= fref / 10000;
	errs	+= ai64ssa_rag_nrate		(args->fd, -1, 1, nrate,						NULL);
	errs	+= ai64ssa_rag_enable		(args->fd, -1, 1, AI64SSA_GEN_ENABLE_YES,		NULL);
	errs	+= ai64ssa_fsamp_ai_report	(args->fd, -1, 1, -1, &sps);
	gsc_label_level_dec();

	//===============================================================
	gsc_label("Auxiliary Outputs");
	printf("\n");
	gsc_label_level_inc();

	errs	+= ai64ssa_ext_sync_enable	(args->fd, -1, 1, AI64SSA_EXT_SYNC_ENABLE_YES,	NULL);
	errs	+= ai64ssa_aux_0_mode		(args->fd, -1, 1, AI64SSA_AUX_MODE_OUTPUT,		NULL);
	errs	+= ai64ssa_aux_1_mode		(args->fd, -1, 1, AI64SSA_AUX_MODE_OUTPUT,		NULL);
	errs	+= ai64ssa_aux_2_mode		(args->fd, -1, 1, AI64SSA_AUX_MODE_OUTPUT,		NULL);
	errs	+= ai64ssa_aux_3_mode		(args->fd, -1, 1, AI64SSA_AUX_MODE_OUTPUT,		NULL);
	errs	+= ai64ssa_aux_noise		(args->fd, -1, 1, AI64SSA_AUX_NOISE_HIGH,		NULL);

	gsc_label_level_dec();

	//===============================================================
	sprintf(buf, "Waiting (%ld sec%s)", (long) args->seconds, (args->seconds == 1) ? "" : "s");
	gsc_label(buf);
	fflush(stdout);
	os_sleep_ms(args->seconds * 1000);
	printf("Done\n");

	return(errs);
}



