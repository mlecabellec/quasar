// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/savedata/main.c $
// $Rev: 54968 $
// $Date: 2024-08-07 15:57:37 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



// macros *********************************************************************

#define	AI_DIFF			AI64SSA_AI_MODE_DIFF
#define	AI_PS_DIFF		AI64SSA_AI_MODE_PS_DIFF
#define	AI_SINGLE		AI64SSA_AI_MODE_SINGLE
#define	AI_VREF			AI64SSA_AI_MODE_VREF
#define	AI_ZERO			AI64SSA_AI_MODE_ZERO

#define	CH_0_1			AI64SSA_CHAN_ACTIVE_0_1
#define	CH_0_3			AI64SSA_CHAN_ACTIVE_0_3
#define	CH_0_7			AI64SSA_CHAN_ACTIVE_0_7
#define	CH_0_15			AI64SSA_CHAN_ACTIVE_0_15
#define	CH_0_31			AI64SSA_CHAN_ACTIVE_0_31
#define	CH_0_63			AI64SSA_CHAN_ACTIVE_0_63
#define	CH_RANGE		AI64SSA_CHAN_ACTIVE_RANGE
#define	CH_SINGLE		AI64SSA_CHAN_ACTIVE_SINGLE

#define	VR_2_5			AI64SSA_AI_RANGE_2_5V
#define	VR_5			AI64SSA_AI_RANGE_5V
#define	VR_10			AI64SSA_AI_RANGE_10V
#define	VR_0_5			AI64SSA_AI_RANGE_0_5V
#define	VR_0_10			AI64SSA_AI_RANGE_0_10V



//*****************************************************************************
static int _parse_args(int argc, char** argv, args_t* args)
{
	const gsc_arg_item_t	list[]	=
	{
		//	type				var						arg			values					desc
		{	GSC_ARG_S32_FLAG,	&args->chan_active,		"-ac01",	{ CH_0_1 },				"Active Channel Selection: channels 0 through 1."					},
		{	GSC_ARG_S32_FLAG,	&args->chan_active,		"-ac03",	{ CH_0_3 },				"Active Channel Selection: channels 0 through 3."					},
		{	GSC_ARG_S32_FLAG,	&args->chan_active,		"-ac07",	{ CH_0_7 },				"Active Channel Selection: channels 0 through 7."					},
		{	GSC_ARG_S32_FLAG,	&args->chan_active,		"-ac015",	{ CH_0_15 },			"Active Channel Selection: channels 0 through 15."					},
		{	GSC_ARG_S32_FLAG,	&args->chan_active,		"-ac031",	{ CH_0_31 },			"Active Channel Selection: channels 0 through 31."					},
		{	GSC_ARG_S32_FLAG,	&args->chan_active,		"-ac063",	{ CH_0_63 },			"Active Channel Selection: channels 0 through 63."					},
		{	GSC_ARG_S32_FLAG,	&args->chan_active,		"-acr",		{ CH_RANGE },			"Active Channel Selection: channel Range (requires -fc# and -lc#)."	},
		{	GSC_ARG_S32_FLAG,	&args->chan_active,		"-acs",		{ CH_SINGLE },			"Active Channel Selection: Single channel (requires -sc#)."			},
		{	GSC_ARG_S32_FLAG,	&args->io_mode,			"-bmdma",	{ GSC_IO_MODE_BMDMA },	"Transfer data using Block Mode DMA."								},
		{	GSC_ARG_S32_FLAG,	&args->continuous,		"-c",		{ -1 },					"Continue testing until an error occurs."							},
		{	GSC_ARG_S32_FLAG,	&args->continuous,		"-C",		{ +1 },					"Continue testing even if errors occur."							},
		{	GSC_ARG_S32_FLAG,	&args->chan_tag,		"-cte",		{ CHAN_TAG_EXCLUDE },	"Exclude the Channel Tag from the saved data."						},
		{	GSC_ARG_S32_FLAG,	&args->chan_tag,		"-cto",		{ CHAN_TAG_ONLY },		"Include only the Channel Tag in the saved data."					},
		{	GSC_ARG_S32_FLAG,	&args->format,			"-dec",		{ FORMAT_DEC },			"Save data as decimal numbers."										},
		{	GSC_ARG_S32_FLAG,	&args->format,			"-dec0",	{ FORMAT_DEC_0 },		"Center decimal data around zero instead of mid scale."				},
		{	GSC_ARG_S32_FLAG,	&args->ai_mode,			"-diff",	{ AI_DIFF },			"Select Differential signal processing."							},
		{	GSC_ARG_S32_FLAG,	&args->io_mode,			"-dmdma",	{ GSC_IO_MODE_DMDMA },	"Transfer data using Demand Mode DMA."								},
		{	GSC_ARG_S32_FLAG,	&args->force_save,		"-f",		{ 1 },					"Force data to be saved even if errors occur."						},
		{	GSC_ARG_S32_RANGE,	&args->chan_first,		"-fc",		{ 0, 63 },				"Use First Channel number # (requires -acr)."						},
		{	GSC_ARG_S32_RANGE,	&args->fsamp,			"-fsamp",	{ 1, 200000 },			"Sample data at # samples per second (default is 10,000)."			},
		{	GSC_ARG_S32_FLAG,	&args->format,			"-hex",		{ 0 },					"Save data in hexadecimal format (default)."						},
		{	GSC_ARG_S32_RANGE,	&args->chan_last,		"-lc",		{ 0, 63 },				"Use Last Channel number # (requires -acr)."						},
		{	GSC_ARG_S32_MIN,	&args->minute_limit,	"-m",		{ 1 },					"Limit continuous testing to # minutes."							},
		{	GSC_ARG_S32_MIN,	&args->test_limit,		"-n",		{ 1 },					"Limit continuous testing to # iterations."							},
		{	GSC_ARG_S32_FLAG,	&args->ai_mode,			"-pdiff",	{ AI_PS_DIFF },			"Select Pseudo Differential signal processing."						},
		{	GSC_ARG_S32_FLAG,	&args->io_mode,			"-pio",		{ GSC_IO_MODE_PIO },	"Transfer data using PIO (the default)."							},
		{	GSC_ARG_S32_RANGE,	&args->repeat,			"-r",		{ 1, 10000 },			"Repeat data retreaval # times before saving the data."				},
		{	GSC_ARG_S32_RANGE,	&args->chan_single,		"-sc",		{ 0, 63 },				"Use Single Channel number # (requires -acs)."						},
		{	GSC_ARG_S32_FLAG,	&args->ai_mode,			"-se",		{ AI_SINGLE },			"Select Single Ended signal processing (default)."					},
		{	GSC_ARG_S32_FLAG,	&args->v_range,			"-vr2.5",	{ VR_2_5 },				"Select the Voltage Range of +-2.5 volt range."						},
		{	GSC_ARG_S32_FLAG,	&args->v_range,			"-vr5",		{ VR_5 },				"Select the Voltage Range of +-5 volt range."						},
		{	GSC_ARG_S32_FLAG,	&args->v_range,			"-vr10",	{ VR_10 },				"Select the Voltage Range of +-10 volt range (default)."			},
		{	GSC_ARG_S32_FLAG,	&args->v_range,			"-vr05",	{ VR_0_5 },				"Select the Voltage Range of 0 to +5 volt range."					},
		{	GSC_ARG_S32_FLAG,	&args->v_range,			"-vr010",	{ VR_0_10 },			"Select the Voltage Range of 0 to +10 volt range."					},
		{	GSC_ARG_S32_FLAG,	&args->ai_mode,			"-vref",	{ AI_VREF },			"Connect the signal inputs to the +VREF voltage reference."			},
		{	GSC_ARG_S32_FLAG,	&args->ai_mode,			"-zero",	{ AI_ZERO },			"Connect the signal inputs to internal ground reference."			},
		{	GSC_ARG_DEV_INDEX,	&args->index,			"index",	{ 1 },					"The zero based index of the device to access."						}
	};

	const gsc_arg_set_t	set	=
	{
		/* name	*/	"savedata",
		/* desc	*/	"Capture and save data to disk.",
		/* qty	*/	SIZEOF_ARRAY(list),
		/* list	*/	list
	};

	int	errs;

	memset(args, 0, sizeof(args_t));
	args->argc			= argc;
	args->argv			= argv;
	args->ai_mode		= AI_SINGLE;
	args->chan_active	= -1;				// select the maximum
	args->chan_first	= -1;				// select the minimum
	args->chan_last		= -1;				// select the maximum
	args->fsamp			= 10000;
	args->io_mode		= GSC_IO_MODE_PIO;
	args->repeat		= 1;
	args->v_range		= AI64SSA_AI_RANGE_10V;

	errs	= gsc_args_parse(argc, argv, &set);

	if ((args->format == FORMAT_DEC_0) && (args->chan_tag != CHAN_TAG_EXCLUDE))
	{
		errs++;
		printf("FAIL <---  (-dec0 requires -cte)\n");
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
	int		failures	= 0;
	long	hours;
	long	mins;
	time_t	now;
	int		passes		= 0;
	int		ret;
	long	secs;
	time_t	t_limit;
	time_t	test;
	int		tests		= 0;

	for (;;)	// A convenience loop.
	{
		gsc_label_init(30);
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


