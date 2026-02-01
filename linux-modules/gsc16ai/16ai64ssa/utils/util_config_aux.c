// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_config_aux.c $
// $Rev: 53557 $
// $Date: 2023-08-07 14:30:23 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_config_aux
*
*	Purpose:
*
*		Configure the Auxiliary services.
*
*	Arguments:
*
*		fd		The handle to use to access the driver.
*
*		index	The index of the device to access. Ignore if < 0.
*				This is for display purposes only.
*
*		verbose	Work verbosely?
*
*		set		The setting to apply.
*
*		get		The current setting is recorded here, if not NULL.
*
*	Returned:
*
*		>= 0	The number of errors encountered here.
*
******************************************************************************/

int	ai64ssa_config_aux(int fd, int index, int verbose)
{
	int	errs	= 0;

	if (verbose)
	{
		gsc_label_index("Auxiliary Configuration", index);
		printf("\n");
		gsc_label_level_inc();
	}

	// Settings must be applied after the initialization call!

	errs	+= ai64ssa_aux_in_pol	(fd, index, verbose, AI64SSA_AUX_IN_POL_LO_2_HI,	NULL);
	errs	+= ai64ssa_aux_noise	(fd, index, verbose, AI64SSA_AUX_NOISE_HIGH,		NULL);
	errs	+= ai64ssa_aux_out_pol	(fd, index, verbose, AI64SSA_AUX_OUT_POL_HI_PULSE,	NULL);
	errs	+= ai64ssa_aux_0_mode	(fd, index, verbose, AI64SSA_AUX_MODE_DISABLE,		NULL);
	errs	+= ai64ssa_aux_1_mode	(fd, index, verbose, AI64SSA_AUX_MODE_DISABLE,		NULL);
	errs	+= ai64ssa_aux_2_mode	(fd, index, verbose, AI64SSA_AUX_MODE_DISABLE,		NULL);
	errs	+= ai64ssa_aux_3_mode	(fd, index, verbose, AI64SSA_AUX_MODE_DISABLE,		NULL);

	if (verbose)
		gsc_label_level_dec();

	return(errs);
}


