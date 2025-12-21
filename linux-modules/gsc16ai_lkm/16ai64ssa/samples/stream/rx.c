// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/stream/rx.c $
// $Rev: 54970 $
// $Date: 2024-08-07 15:58:34 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



// variables ******************************************************************

static	os_time_ns_t	_start_time;
static	int				_start_time_set	= 0;



//*****************************************************************************
static void _wait_til_done(const rx_data_t* rx)
{
	// Wait until the processing thread indicates that it is done.

	for (;;)
	{
		if (rx->done)
			break;

		gsc_time_sleep_ms(1);
	}
}



//*****************************************************************************
static void _wait_til_started(const rx_data_t* rx)
{
	// Wait until the processing thread indicates that it has started.

	for (;;)
	{
		if ((rx->started) || (rx->stop))
			break;

		gsc_time_sleep_ms(1);
	}
}



//*****************************************************************************
static void _wait_for_start(const rx_data_t* rx)
{
	// Wait until the controlling thread tells us to start working.

	for (;;)
	{
		if ((rx->start) || (rx->stop))
			break;

		gsc_time_sleep_ms(1);
	}
}



//*****************************************************************************
// Clear the input buffer, then read in data til good data appears.
static int _flush_input_buffer(const args_t* args)
{
	int			errs;
	rx_data_t*	rx		= args->rx;

	errs	= ai64ssa_ai_buf_clear(args->fd, -1, 0);

	if (errs)
	{
		errs		= 1;
		rx->errs	= 1;
		sprintf(rx->err_buf, "Input Buffer clear: failed");
	}

	return(errs);
}



//*****************************************************************************
static void _check_eof(
	const args_t*	args,
	long long		byte_limit,
	struct timeval*	time_limit,
	gsc_buf_man_t*	bm)
{
	struct timeval	now;
	rx_data_t*		rx	= args->rx;

	if (args->rx_seconds)
	{
		gettimeofday(&now, NULL);

		if (now.tv_sec > time_limit->tv_sec)
		{
			bm->eof	= 1;
		}
		else if (	(now.tv_sec == time_limit->tv_sec) &&
					(now.tv_usec >= time_limit->tv_usec))
		{
			bm->eof	= 1;
		}
	}
	else if (rx->total_bytes >= byte_limit)
	{
		bm->eof	= 1;
	}
}



//*****************************************************************************
static void _compute_timestamp(const args_t* args, gsc_buf_man_t* bm)
{
	s32					chan_samp_qty;
	os_time_ns_t		now;
	long				ns;
	double				read_period;
	const rx_data_t*	rx			= args->rx;
	s32					sample_qty;
	double				sample_time;
	double				secs;
	os_time_ns_t		timestamp;
	bm_user_area_t*		user_area;

	if (args->context_info)
	{
		if (bm->count)
			sample_time	= (double) 1.0 / rx->fsamp;
		else
			sample_time	= 0;

		os_time_get_ns(&now);
		sample_qty			= bm->count / 4;
		chan_samp_qty		= sample_qty / rx->chan_qty;
		read_period			= sample_time * chan_samp_qty;
		timestamp			= now;
		secs				= floor(read_period);
		ns					= (read_period - secs) * 1000000000;

		// Move one second of time from tv_sec to tv_nsec.
		timestamp.tv_sec	-= 1;
		timestamp.tv_nsec	+= 1000000000;
		timestamp.tv_sec	-= secs;

		timestamp.tv_nsec	-= ns;
		timestamp.tv_nsec	+= 500000;	// ms round up
		timestamp.tv_nsec	-= timestamp.tv_nsec % 1000000;	// drop us & ns accuracy

		if (timestamp.tv_nsec > 1000000000)
		{
			timestamp.tv_sec	+= 1;
			timestamp.tv_nsec	-= 1000000000;
		}

		// Record timestamp for Tx thread
		user_area				= (void*) bm->user_area;
		user_area->timestamp	= timestamp;
	}
}



//*****************************************************************************
static void _rx_process(const args_t* args)
{
	struct timeval	begin;
	gsc_buf_man_t	bm;
	long long		byte_limit	= 1000000LL * args->rx_mb;
	int				eof;
	int				err;
	int				errs;
	int				ret;
	rx_data_t*		rx			= args->rx;
	struct timeval	time_limit;

	memset(&bm, 0, sizeof(gsc_buf_man_t));

	errs	= _flush_input_buffer(args);

	if (errs)
	{
		bm.eof		= 1;
		rx->errs	= 1;
	}

	gettimeofday(&begin, NULL);
	time_limit.tv_sec	= begin.tv_sec + args->rx_seconds;
	time_limit.tv_usec	= begin.tv_usec;

	if (sizeof(bm_user_area_t) > BUF_MAN_USER_AREA_SIZE)
	{
		errs		= 1;
		bm.eof		= 1;
		rx->errs	= 1;
		strcpy(rx->err_buf, "sizeof(bm_user_area_t) > BUF_MAN_USER_AREA_SIZE");
	}

	rx->total_ms	= gsc_time_delta_ms();

	for (; errs == 0;)
	{
 		rx->reading		= 1;	// Let Tx thread know we've started reading data.

		// Request an empty buffer so we can fill it.
		errs	= gsc_buf_man_request_empty(&bm);

		if (rx->stop)
		{
			bm.eof	= 1;
			gsc_buf_man_release_buffer(&bm);
			break;
		}

		// Perform validation.

		if (errs)
		{
			bm.eof		= 1;
			rx->errs	= 1;
			strcpy(rx->err_buf, "Rx empty request: failed");
			gsc_buf_man_release_buffer(&bm);
			break;
		}

		if (bm.buffer == NULL)
		{
			bm.eof		= 1;
			rx->errs	= 1;
			strcpy(rx->err_buf, "Rx empty request: NULL buffer");
			gsc_buf_man_release_buffer(&bm);
			break;
		}

		if (bm.size == 0)
		{
			bm.eof		= 1;
			rx->errs	= 1;
			strcpy(rx->err_buf, "Rx empty request: zero sized buffer");
			gsc_buf_man_release_buffer(&bm);
			break;
		}

		switch (args->rx_option)
		{
			default:

				bm.eof		= 1;
				rx->errs	= 1;
				sprintf(rx->err_buf, "Rx option invalid: %d", (int) args->rx_option);
				break;

			case RX_OPTION_ZERO_DATA:

				memset(bm.buffer, 0, bm.size);
				bm.offset		= 0;
				bm.count		= bm.size - (bm.size % 4);
				rx->total_bytes	+= bm.count;
				_check_eof(args, byte_limit, &time_limit, &bm);
				break;

			case RX_OPTION_READ_DEV:

				bm.offset	= 0;
				bm.count	= bm.size - (bm.size % 4);
				ret			= ai64ssa_read(args->fd, bm.buffer, bm.count);

				if (ret < 0)
				{
					bm.eof		= 1;
					bm.count	= 0;
					rx->errs	= 1;
					sprintf(rx->err_buf, "Rx read failure: %d", (int) ret);
					break;
				}

				_compute_timestamp(args, &bm);

				bm.count		= ret;
				rx->total_bytes	+= ret;
				_check_eof(args, byte_limit, &time_limit, &bm);

				if (bm.eof)
				{
					// Disable input to the FIFO.
					errs	= ai64ssa_rag_enable(args->fd, -1, 0, AI64SSA_GEN_ENABLE_NO, NULL);

					if (errs)
					{
						rx->errs	= 1;
						strcpy(rx->err_buf, "Rate-A Generator disable failure");
					}

					errs	= ai64ssa_rbg_enable(args->fd, -1, 0, AI64SSA_GEN_ENABLE_NO, NULL);

					if (errs)
					{
						rx->errs	= 1;
						strcpy(rx->err_buf, "Rate-B Generator disable failure");
					}
				}

				break;
		}

		if ((errs) || (rx->errs))
		{
			bm.eof		= 1;
			rx->errs	= 1;
			rx->stop	= 1;
		}

		eof	= bm.eof;
		err	= gsc_buf_man_release_buffer(&bm);

		if ((err) && (rx->errs == 0))
		{
			rx->errs	= 1;
			rx->stop	= 1;
			strcpy(rx->err_buf, "Rx buffer release failure");
		}

		if ((eof) || (rx->errs) || (rx->stop))
			break;
	}

	rx->total_ms	= gsc_time_delta_ms() - rx->total_ms;
}



//*****************************************************************************
static int _rx_thread(void* arg)
{
	const args_t*	args	= arg;
	rx_data_t*		rx		= args->rx;

	// Tell the controlling code that this thread has started.
	rx->started	= 1;

	// Wait till we're told to start.
	_wait_for_start(rx);

	// Perform the expected activity.
	_rx_process(args);

	// Tell the controlling code that we're done.
	rx->done	= 1;
	return(0);
}



//*****************************************************************************
int rx_start(const args_t* args)
{
	char		buf[1024]	= "";
	int			errs		= 0;
	rx_data_t*	rx			= args->rx;

	gsc_label("Rx Startup");

	if (_start_time_set == 0)
	{
		os_time_get_ns(&_start_time);
		_start_time_set	= 1;
	}

	rx->start_time	= _start_time;

	switch (args->rx_option)
	{
		default:

			errs	= 1;
			strcpy(buf, "invalid option");
			break;

		case RX_OPTION_ZERO_DATA:

			strcpy(buf, "providing NULL data");
			break;

		case RX_OPTION_READ_DEV:

			strcpy(buf, "reading from device");
			break;
	}

	if (args->rx_seconds)
	{
		strcat(buf, ", >= ");
		gsc_label_long_comma_buf(args->rx_seconds, buf + strlen(buf));
		strcat(buf, " Second");

		if (args->rx_seconds != 1)
			strcat(buf, "s");
	}
	else
	{
		strcat(buf, ", >= ");
		gsc_label_long_comma_buf(args->rx_mb, buf + strlen(buf));
		strcat(buf, " MB");
	}

	if (errs == 0)
	{
		errs	= os_thread_create(&rx->thread, "Rx Thread", _rx_thread, (void*) args);

		if (errs)
			strcpy(buf, "creating Rx thread");
	}

	if (errs)
	{
		printf("FAIL <---  (%s)\n", buf);
	}
	else
	{
		_wait_til_started(rx);
		printf("PASS  (%s)\n", buf);
	}

	return(errs);
}



//*****************************************************************************
int rx_stop(const args_t* args)
{
	int			errs;
	rx_data_t*	rx		= args->rx;

	// STOP ===============================================
	gsc_label("Rx Stop");
	rx->stop	= 1;
	ai64ssa_rx_io_abort(args->fd, -1, 0, NULL);

	if (rx->started)
		_wait_til_done(rx);

	os_thread_destroy(&rx->thread);
	printf("Done\n");

	gsc_label_level_inc();

	// STATUS =============================================
	gsc_label("Status");

	if (rx->errs == 0)
	{
		errs	= 0;
		printf("PASS\n");
	}
	else
	{
		errs	= 1;
		printf("FAIL <---");

		if (rx->err_buf[0])
			printf(" (%s)", rx->err_buf);

		printf("\n");
	}

	// THROUGHPUT =========================================
	gsc_label("Throughput");
	gsc_label_long_comma(rx->total_bytes / 4);
	printf(" samples, ");
	gsc_label_long_comma(rx->total_ms / 1000);
	printf(".%03ld Secs", (long) rx->total_ms % 1000);

	if (rx->total_ms)
	{
		printf(", ");
		gsc_label_long_comma(1000LL * rx->total_bytes / 4 / rx->total_ms);
		printf(" S/S");
	}

	printf("\n");

	gsc_label_level_dec();
	return(errs);
}


