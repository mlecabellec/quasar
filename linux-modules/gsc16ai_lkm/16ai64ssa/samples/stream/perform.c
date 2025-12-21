// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/stream/perform.c $
// $Rev: 54970 $
// $Date: 2024-08-07 15:58:34 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _buffer_config(void)
{
	int		errs;

	gsc_label("Buffer Configuration");
	errs	= gsc_buf_man_init();

	if (errs == 0)
		errs	= gsc_buf_man_setup(BUFFER_QTY, BUFFER_SIZE);

	if (errs)
	{
	}
	else
	{
		printf("PASS  (%d buffers, ", (int) BUFFER_QTY);
		gsc_label_long_comma(BUFFER_SIZE);
		printf(" bytes each)\n");
	}

	return(errs);
}



//*****************************************************************************
static void _buffer_free(void)
{
	gsc_label("Buffer Free");
	gsc_buf_man_free_all();
	printf("Done\n");
}



//*****************************************************************************
static void _buffer_stats(void)
{
	gsc_buf_man_stats();
}



//*****************************************************************************
static void _buffer_stop(void)
{
	gsc_buf_man_stop();
}



//*****************************************************************************
static int _config_device(const args_t* args)
{
	s32			chan_qty	= 32;
	int			errs		= 0;
	rx_data_t*	rx			= args->rx;
	s32			secs;
	tx_data_t*	tx			= args->tx;

	gsc_label("Configuration");
	printf("\n");
	gsc_label_level_inc();

	gsc_label("Configure");
	errs	+= ai64ssa_config_ai			(args->fd, -1, 0, args->fsamp);
	printf("%s\n", errs ? "FAIL <---" : "PASS");

	errs	+= ai64ssa_rx_io_mode			(args->fd, -1, 1, args->io_mode, NULL);
	errs	+= ai64ssa_fsamp_ai_compute		(args->fd, -1, 1, args->fsamp, NULL, NULL, NULL, NULL, NULL, NULL, &rx->fsamp);
	errs	+= ai64ssa_fsamp_ai_report_all	(args->fd, -1, 1, NULL);

	// Compute a timeout period based on the sample rate.
	errs	+= ai64ssa_query				(args->fd, -1, 1, AI64SSA_QUERY_CHANNEL_QTY, &chan_qty);
	secs	= (s32) (((float) BUFFER_SIZE / chan_qty) / rx->fsamp + 5.5);
	errs	+= ai64ssa_rx_io_timeout		(args->fd, -1, 1, secs, NULL);

	gsc_label_level_dec();

	rx->chan_qty		= chan_qty;
	tx->chan_qty		= chan_qty;
	tx->file_samples	= 1024L * 1024L * args->file_samples;

	return(errs);
}



//*****************************************************************************
static int _work_start(const args_t* args, int errs)
{
	char		buf[80];
	rx_data_t*	rx		= args->rx;
	tx_data_t*	tx		= args->tx;

	strcpy(buf, "Working");

	if (args->force)
	{
		errs	= 0;
		strcat(buf, " (Forced)");
	}

	gsc_label(buf);

	if (errs)
	{
		printf("ABORTED DUE TO SETUP ERRORS <---\n");
		errs	= 0;
	}
	else
	{
		printf("\n");
		gsc_label_level_inc();

		// Begin working.
		gsc_label("Starting");
		tx->start	= 1;
		rx->start	= 1;
		printf("Done\n");

		gsc_label("Waiting");

		for (;;)
		{
			if ((rx->errs) || (tx->errs))
			{
				tx->stop	= 1;
				rx->stop	= 1;
				_buffer_stop();
			}

			if ((rx->done) && (tx->done))
				break;

			gsc_time_sleep_ms(50);
		}

		_buffer_stop();
		printf("Done\n");

		gsc_label_level_dec();
	}

	return(errs);
}



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int	errs	= 0;

	if ((args->delay_test_s) && (args->tests))
	{
		gsc_label("Delay Between tests");
		printf(	"Test %d, %d Second%s ... ",
				(int) args->tests + 1,
				args->delay_test_s,
				(args->delay_test_s == 1) ? "" : "s");
		fflush(stdout);
		gsc_time_sleep_ms((long) args->delay_test_s * 1000);
		printf("Done\n");
		fflush(stdout);
	}

	errs	+= _config_device(args);
	errs	+= _buffer_config();

	errs	+= rx_start(args);
	errs	+= tx_start(args);

	errs	+= _work_start(args, errs);

	errs	+= rx_stop(args);
	errs	+= tx_stop(args);

	if (args->stats)
		_buffer_stats();

	_buffer_free();

	if (args->tx_validate)
		tx_validation_results();

	errs	+= ai64ssa_ai_buf_overflow	(args->fd, -1, 1, -1, NULL);
	errs	+= ai64ssa_ai_buf_underflow	(args->fd, -1, 1, -1, NULL);

	return(errs);
}


