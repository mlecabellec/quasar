// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/regs/main.c $
// $Rev: 53723 $
// $Date: 2023-09-14 10:40:07 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _parse_args(int argc, char** argv, args_t* args)
{
	const gsc_arg_item_t	list[]	=
	{
		//	type				var				arg			values	desc
		{	GSC_ARG_DEV_INDEX,	&args->index,	"index",	{ 1 },	"The zero based index of the device to access."	}
	};

	const gsc_arg_set_t	set	=
	{
		/* name	*/	"regs",
		/* desc	*/	"Register Access Application",
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
	long	failures	= 0;
	long	hours;
	long	mins;
	time_t	now;
	long	passes		= 0;
	int		ret;
	long	secs;
	long	tests		= 0;

	for (;;)	// A convenience loop.
	{
		gsc_label_init(18);
		errs	= _parse_args(argc, argv, &args);

		// Introduction

		os_id_host();
		_show_time_stamp();
		_show_access_index(&args);
		errs	+= opto16x16_init_util(1);
		errs	+= os_id_driver(opto16x16_open, opto16x16_read, opto16x16_close);
		errs	+= opto16x16_count_boards(1, &args.qty);

		if (args.qty <= 0)
			errs++;

		// Open access to device

		if (errs == 0)
			errs	= opto16x16_open_util(args.index, 0, -1, 1, &args.fd);

		if (errs == 0)
		{
			// Device identification

			errs	= opto16x16_id_device(args.fd, -1, 1);

			// Perform testing

			errs	+= perform_tests(&args);

			// Close device access

			errs	+= opto16x16_close_util(args.fd, -1, 1);
		}

		// End of processing.

		now	= time(NULL);
		tests++;

		if (errs)
		{
			failures++;
			printf(	"\nRESULTS: FAIL <---\n");
		}
		else
		{
			passes++;
			printf("\nRESULTS: PASS\n");
		}

		secs	= (long) (now - exec);
		hours	= secs / 3600;
		secs	= secs % 3600;
		mins	= secs / 60;
		secs	= secs % 60;
		printf(	"SUMMARY: tests %ld, pass %ld, fail %ld"
				" (duration %ld:%ld:%02ld)\n\n",
				tests,
				passes,
				failures,
				hours,
				mins,
				secs);
		break;
	}

	ret	= ((args.qty <= 0) || (errs) || (failures)) ? EXIT_FAILURE : EXIT_SUCCESS;
	return(ret);
}


