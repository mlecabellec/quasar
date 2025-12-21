// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/wait/main.c $
// $Rev: 54969 $
// $Date: 2024-08-07 15:58:15 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _parse_args(int argc, char** argv, args_t* args)
{
	const gsc_arg_item_t	list[]	=
	{
		//	type				var						arg			values	desc
		{	GSC_ARG_S32_FLAG,	&args->continuous,		"-c",		{ -1 },	"Continue testing until an error occurs."		},
		{	GSC_ARG_S32_FLAG,	&args->continuous,		"-C",		{ +1 },	"Continue testing even if errors occur."		},
		{	GSC_ARG_S32_MIN,	&args->minute_limit,	"-m",		{ 1 },	"Continue testing for up to # minutes."			},
		{	GSC_ARG_S32_MIN,	&args->test_limit,		"-n",		{ 1 },	"Continue testing for up to # iterations."		},
		{	GSC_ARG_DEV_INDEX,	&args->index,			"index",	{ 1 },	"The zero based index of the device to access."	}
	};

	const gsc_arg_set_t	set	=
	{
		/* name	*/	"wait",
		/* desc	*/	"Wait sample application.",
		/* qty	*/	SIZEOF_ARRAY(list),
		/* list	*/	list
	};

	int	errs;

	memset(args, 0, sizeof(args_t));

	errs	= gsc_args_parse(argc, argv, &set);

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


