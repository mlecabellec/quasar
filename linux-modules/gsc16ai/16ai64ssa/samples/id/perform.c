// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/id/perform.c $
// $Rev: 53554 $
// $Date: 2023-08-07 14:25:54 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
static void _input_modes(void)
{
	gsc_label("Input Modes");
	printf("Full Differential, Pseudo Differential, Zero Test, +Vref Test\n");
}



//*****************************************************************************
static void _voltage_ranges(void)
{
	gsc_label("Voltage Ranges");
	printf("+-10V, +-5V, +-2.5V, 0-5V, 0-10V\n");
}



//*****************************************************************************
static void _clocking_sources(void)
{
	gsc_label("Clocking Sources");
	printf("Rate Generator(s), External, Software\n");
}



//*****************************************************************************
static void _sampling_modes(void)
{
	gsc_label("Sampling Modes");
	printf("Continuous, Burst\n");
}



//*****************************************************************************
static int _id_device_reg(const args_t* args)
{
	int						errs;
	gsc_reg_def_t			list[2]	= { { NULL }, { NULL } };
	const gsc_reg_def_t*	ptr;

	printf("\n");
	ptr	= ai64ssa_reg_get_def_id(AI64SSA_GSC_BCFGR);

	if (ptr)
	{
		list[0]	= ptr[0];
		errs	= gsc_reg_list(args->fd, list, 1, ai64ssa_reg_read);
	}
	else
	{
		errs	= 1;
		printf(	"FAIL <---  (%d. ai64ssa_reg_get_def_id, AI64SSA_GSC_BCFGR)\n",
				__LINE__);
	}

	return(errs);
}



//*****************************************************************************
static int _register_map(const args_t* args)
{
	int	errs;

	printf("\n");
	errs	= ai64ssa_reg_list(args->fd, args->detail);
	return(errs);
}



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int	errs	= 0;
	int	i;

	gsc_label("Device Features");
	printf("\n");

	gsc_label_level_inc();

	_input_modes();
	_voltage_ranges();
	_clocking_sources();
	_sampling_modes();

	for (i = 0; i < AI64SSA_IOCTL_QUERY_LAST; i++)
		errs	+= ai64ssa_query(args->fd, -1, 1, i, NULL);

	gsc_label_level_dec();

	errs	+= _id_device_reg(args);
	errs	+= _register_map(args);
	printf("\n");

	return(errs);
}


