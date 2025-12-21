// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/stream/main.c $
// $Rev: 54970 $
// $Date: 2024-08-07 15:58:34 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _parse_args(int argc, char** argv, args_t* args)
{
	const gsc_arg_item_t	list[]	=
	{
		//	type				var						arg			values							desc
		{	GSC_ARG_S32_FLAG,	&args->io_mode,			"-bmdma",	{ GSC_IO_MODE_BMDMA },			"Transfer data using Block Mode DMA."						},
//		{	GSC_ARG_S32_FLAG,	&args->continuous,		"-c",		{ -1 },							"Continue testing until an error occurs."					},
//		{	GSC_ARG_S32_FLAG,	&args->continuous,		"-C",		{ +1 },							"Continue testing even if errors occur."					},
		{	GSC_ARG_S32_FLAG,	&args->io_mode,			"-dmdma",	{ GSC_IO_MODE_DMDMA },			"Transfer data using Demand Mode DMA."						},
//		{	GSC_ARG_S32_RANGE,	&args->delay_test_s,	"-dt",		{ 0, 3600 },					"Delay for # seconds between continuous tests."				},
		{	GSC_ARG_S32_FLAG,	&args->force,			"-f",		{ 1 },							"Force operation even if errors occur."						},
		{	GSC_ARG_S32_RANGE,	&args->fsamp,			"-fsamp",	{ 1, 200000 },					"Sample input data at # S/S (default: 5,000)."				},
//		{	GSC_ARG_S32_MIN,	&args->minute_limit,	"-m",		{ 1 },							"Limit continuous testing to # minutes."					},
//		{	GSC_ARG_S32_MIN,	&args->test_limit,		"-n",		{ 1 },							"Limit continuous testing to # iterations."					},
		{	GSC_ARG_S32_FLAG,	&args->io_mode,			"-pio",		{ GSC_IO_MODE_PIO },			"Transfer data using PIO (default)."						},
		{	GSC_ARG_S32_FLAG,	&args->rx_option,		"-rx0",		{ RX_OPTION_ZERO_DATA },		"Provide null data rather than read from the device."		},
		{	GSC_ARG_S32_FLAG,	&args->rx_option,		"-rxd",		{ RX_OPTION_READ_DEV },			"Read data from the device (default)."						},
		{	GSC_ARG_S32_RANGE,	&args->rx_mb,			"-rxmb",	{ 1, 30000 },					"Receive # megabytes of data (default: 5)."					},
		{	GSC_ARG_S32_RANGE,	&args->rx_seconds,		"-rxs",		{ 1, 3600 },					"Provide Rx data for # seconds."							},
		{	GSC_ARG_S32_FLAG,	&args->stats,			"-s",		{ 1 },							"Show transfer statistics upon completion."					},
		{	GSC_ARG_S32_FLAG,	&args->tx_option,		"-tx0",		{ TX_OPTION_BIT_BUCKET },		"Do nothing with the received data (default)."				},
		{	GSC_ARG_S32_FLAG,	&args->tx_option,		"-txb",		{ TX_OPTION_WRITE_FILE_BIN },	"Write the receive data to a binary file (datast.bin)."		},
		{	GSC_ARG_S32_FLAG,	&args->context_info,	"-txci",	{ 1 },							"Add context information to the file name."					},
		{	GSC_ARG_S32_FLAG,	&args->tx_chan_tag,		"-txcte",	{ CHAN_TAG_EXCLUDE },			"Exclude the channel tag from .txt file content."			},
		{	GSC_ARG_S32_FLAG,	&args->tx_chan_tag,		"-txcto",	{ CHAN_TAG_ONLY },				"Save only the channel tag in .txt file content."			},
		{	GSC_ARG_S32_FLAG,	&args->tx_decimal,		"-txd",		{ 1 },							"Save .txt file content in decimal format."					},
		{	GSC_ARG_S32_RANGE,	&args->file_samples,	"-txfs",	{ 0, 30000 },					"Limit files to # million samples (in -r multiples)."		},
		{	GSC_ARG_S32_FLAG,	&args->tx_decimal,		"-txh",		{ 0 },							"Save .txt file content as hexadecimal format (default)."	},
		{	GSC_ARG_S32_FLAG,	&args->tx_option,		"-txt",		{ TX_OPTION_WRITE_FILE_TEXT },	"Write the receive data to a text file (datast.txt)."		},
		{	GSC_ARG_S32_FLAG,	&args->tx_validate,		"-txv",		{ 1 },							"Enable minor validation and reporting of Tx process data."	},
		{	GSC_ARG_DEV_INDEX,	&args->index,			"index",	{ 1 },							"The zero based index of the device to access."				}
	};

	const gsc_arg_set_t	set	=
	{
		/* name	*/	"stream",
		/* desc	*/	"Stream data from the device to disk.",
		/* qty	*/	SIZEOF_ARRAY(list),
		/* list	*/	list
	};

	int	errs;

	memset(args, 0, sizeof(args_t));
	args->fsamp			= 5000;		// keep relatively low for 64 channels using PIO
	args->io_mode		= GSC_IO_MODE_PIO;
	args->rx_mb			= 5;
	args->rx_option		= RX_OPTION_READ_DEV;
	args->tx_option		= TX_OPTION_BIT_BUCKET;

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
	args_t		args;
	int			errs;
	time_t		exec		= time(NULL);
	int			failures	= 0;
	long		hours;
	long		mins;
	time_t		now;
	int			passes		= 0;
	int			ret;
	rx_data_t	rx;
	long		secs;
	time_t		t_limit;
	time_t		test;
	int			tests		= 0;
	tx_data_t	tx;

	for (;;)	// A convenience loop.
	{
		gsc_label_init(29);
		test	= time(NULL);
		errs	= _parse_args(argc, argv, &args);

		memset(&rx, 0, sizeof(rx));
		memset(&tx, 0, sizeof(tx));
		args.rx		= &rx;
		args.tx		= &tx;
		args.tests	= tests;

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


