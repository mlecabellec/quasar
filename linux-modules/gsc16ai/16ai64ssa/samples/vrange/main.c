// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/vrange/main.c $
// $Rev: 54962 $
// $Date: 2024-08-07 15:54:00 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _parse_args(int argc, char** argv, args_t* args)
{
	const gsc_arg_item_t	list[]	=
	{
		//	type				var						arg			values							desc
		{	GSC_ARG_S32_FLAG,	&args->ascii_graph,		"-ag",		{ 80 },							"Equivalent to -ag80."															},
		{	GSC_ARG_S32_RANGE,	&args->ascii_graph,		"-ag",		{ 20, 256 },					"Produce an ASCII graph of the data (20-256, requires -aic#, -aimX and -aivX)."	},
		{	GSC_ARG_S32_RANGE,	&args->ai_chan,			"-aic",		{ 0, 63 },						"Report ONLY data for Analog Input Channel # (0 - 63)."							},
		{	GSC_ARG_S32_FLAG,	&args->ai_mode,			"-aimd",	{ AI64SSA_AI_MODE_DIFF },		"Select ONLY Analog Input Mode: Full Differential."								},
		{	GSC_ARG_S32_FLAG,	&args->ai_mode,			"-aimp",	{ AI64SSA_AI_MODE_PS_DIFF },	"Select ONLY Analog Input Mode: Pseudo Differential."							},
		{	GSC_ARG_S32_FLAG,	&args->ai_mode,			"-aims",	{ AI64SSA_AI_MODE_SINGLE },		"Select ONLY Analog Input Mode: Single Ended."									},
		{	GSC_ARG_S32_FLAG,	&args->ai_mode,			"-aimv",	{ AI64SSA_AI_MODE_VREF },		"Select ONLY Analog Input Mode: Vref."											},
		{	GSC_ARG_S32_FLAG,	&args->ai_mode,			"-aimz",	{ AI64SSA_AI_MODE_ZERO },		"Select ONLY Analog Input Mode: Zero Voltage."									},
		{	GSC_ARG_S32_FLAG,	&args->ai_voltage,		"-aiv2.5",	{ AI64SSA_AI_RANGE_2_5V },		"Select ONLY Analog Input Voltage Range: +-2.5 Volts."							},
		{	GSC_ARG_S32_FLAG,	&args->ai_voltage,		"-aiv5",	{ AI64SSA_AI_RANGE_5V },		"Select ONLY Analog Input Voltage Range: +-5 Volts."							},
		{	GSC_ARG_S32_FLAG,	&args->ai_voltage,		"-aiv10",	{ AI64SSA_AI_RANGE_10V },		"Select ONLY Analog Input Voltage Range: +-10 Volts."							},
		{	GSC_ARG_S32_FLAG,	&args->ai_voltage,		"-aiv05",	{ AI64SSA_AI_RANGE_0_5V },		"Select ONLY Analog Input Voltage Range: 0 - 5 Volts."							},
		{	GSC_ARG_S32_FLAG,	&args->ai_voltage,		"-aiv010",	{ AI64SSA_AI_RANGE_0_10V },		"Select ONLY Analog Input Voltage Range: 0 - 10 Volts."							},
		{	GSC_ARG_S32_FLAG,	&args->continuous,		"-c",		{ -1 },							"Continue testing until an error occurs."										},
		{	GSC_ARG_S32_FLAG,	&args->continuous,		"-C",		{ +1 },							"Continue testing even if errors occur."										},
		{	GSC_ARG_S32_MIN,	&args->minute_limit,	"-m",		{ 1 },							"Continue testing for up to # minutes."											},
		{	GSC_ARG_S32_MIN,	&args->test_limit,		"-n",		{ 1 },							"Continue testing for up to # iterations."										},
		{	GSC_ARG_S32_FLAG,	&args->reg_list,		"-r",		{ 1 },							"Include a register list after processing configuration."						},
		{	GSC_ARG_S32_FLAG,	&args->save,			"-s",		{ 1 },							"Save data to vrange.txt (requires -aimX and -aivX)."							},
		{	GSC_ARG_S32_FLAG,	&args->verbose,			"-v",		{ 1 },							"Enable verbose output."														},
		{	GSC_ARG_DEV_INDEX,	&args->index,			"index",	{ 1 },							"The zero based index of the device to access."									}
	};

	const gsc_arg_set_t	set	=
	{
		/* name	*/	"vrange",
		/* desc	*/	"voltage range test for all input configurations.",
		/* qty	*/	SIZEOF_ARRAY(list),
		/* list	*/	list
	};

	int	errs;

	memset(args, 0, sizeof(args_t));
	args->ai_chan		= -1;
	args->ai_mode		= -1;
	args->ai_voltage	= -1;

	errs	= gsc_args_parse(argc, argv, &set);

	if (errs)
	{
	}
	else if (args->ascii_graph <= 0)
	{
	}
	else if (args->ai_chan < 0)
	{
		errs++;
		printf("ERROR: -aicX argument missing, required by -ag/ag# argument\n");
	}
	else if (args->ai_mode < 0)
	{
		errs++;
		printf("ERROR: -aimX argument missing, required by -ag/ag# argument\n");
	}
	else if (args->ai_voltage < 0)
	{
		errs++;
		printf("ERROR: -aiv# argument missing, required by -ag/ag# argument\n");
	}

	if (errs)
	{
	}
	else if (args->save <= 0)
	{
	}
	else if (args->ai_mode < 0)
	{
		errs++;
		printf("ERROR: -aimX argument missing, required by -s argument\n");
	}
	else if (args->ai_voltage < 0)
	{
		errs++;
		printf("ERROR: -aiv# argument missing, required by -s argument\n");
	}

	return(errs);
}



//*****************************************************************************
static void _show_access_index(const args_t* args)
{
	gsc_label("Accessing Device");
	printf("%d\n", args->index);
}



//*****************************************************************************
static void _show_time_stamp(void)
{
	const char*	psz;
	struct tm*	stm;
	time_t		tt;

	time(&tt);
	stm	= localtime(&tt);
	psz	= (char*) asctime(stm);
	gsc_label("Time Stamp");
	printf("%s", psz);
}



/******************************************************************************
*
*	Function:	main
*
*	Purpose:
*
*		Control the overall flow of the application.
*
*	Arguments:
*
*		argc			The number of command line arguments.
*
*		argv			The list of command line arguments.
*
*	Returned:
*
*		EXIT_SUCCESS	No errors were encounterred.
*		EXIT_FAILURE	One or more problems were encounterred.
*
******************************************************************************/

int main(int argc, char** argv)
{
	args_t	args;
	int		errs;
	time_t	exec		= time(NULL);
	int 	failures	= 0;
	long	hours;
	long	mins;
	time_t	now;
	int 	passes		= 0;
	int		ret;
	long	secs;
	time_t	t_limit;
	time_t	test;
	int 	tests		= 0;

	for (;;)	// A convenience loop.
	{
		gsc_label_init(28);
		test	= time(NULL);
		errs	= _parse_args(argc, argv, &args);

		// Introduction

		t_limit	= exec + (args.minute_limit * 60);
		os_id_host();
		_show_time_stamp();
		_show_access_index(&args);
		errs	+= ai64ssa_init_util(1);
		errs	+= os_id_driver(ai64ssa_open, ai64ssa_read, ai64ssa_close);
		errs	+= ai64ssa_count_boards(1, &args.qty);

		if (args.qty <= 0)
			errs++;

		// Open access to device

		if (errs == 0)
			errs	= ai64ssa_open_util(args.index, 0, -1, 1, &args.fd);

		if (errs)
			args.continuous	= 0;

		if (errs == 0)
		{
			// Device identification

			errs	= ai64ssa_id_device(args.fd, -1, 1);

			// Perform testing

			errs	+= perform_tests(&args);

			// Close device access

			errs	+= ai64ssa_close_util(args.fd, -1, 1);
		}

		// End of processing.

		now	= time(NULL);
		tests++;

		if (errs)
		{
			failures++;
			printf(	"\nRESULTS: FAIL <---  (%d error%s)",
					errs,
					(errs == 1) ? "" : "s");
		}
		else
		{
			passes++;
			printf("\nRESULTS: PASS");
		}

		secs	= (long) (now - test);
		hours	= secs / 3600;
		secs	= secs % 3600;
		mins	= secs / 60;
		secs	= secs % 60;
		printf(" (duration %ld:%ld:%02ld)\n", hours, mins, secs);

		secs	= (long) (now - exec);
		hours	= secs / 3600;
		secs	= secs % 3600;
		mins	= secs / 60;
		secs	= secs % 60;
		printf(	"SUMMARY: tests %d, pass %d, fail %d"
				" (duration %ld:%ld:%02ld)\n\n",
				tests,
				passes,
				failures,
				hours,
				mins,
				secs);

		if (args.continuous == 0)
			break;

		if ((args.continuous < 0) && (errs))
			break;

		if ((args.minute_limit > 0) && (now >= t_limit))
			break;

		if ((args.test_limit > 0) && (tests >= args.test_limit))
			break;
	}

	ret	= ((args.qty <= 0) || (errs) || (failures)) ? EXIT_FAILURE : EXIT_SUCCESS;
	return(ret);
}


