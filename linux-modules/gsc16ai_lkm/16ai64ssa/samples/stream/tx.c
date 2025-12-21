// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/stream/tx.c $
// $Rev: 54970 $
// $Date: 2024-08-07 15:58:34 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



// macros *********************************************************************

#define	MAX_RECORDS	10240
#define	TAG_DECODE(v)		(((v) & 0x10000) ? 1 : 0)
#define	TAG_MASK			0x10000
#define	UNUSED_MASK			0xFFFE0000
#define	VALUE_MASK			0xFFFF



// variables ******************************************************************

static	int		_file_number;
static	long	_file_samples;
static	int		_records		= 0;

static struct
{
	long	bm_count;
	long	chan_0_tags;
	long	err_values;
	long	non_zero_data;
} _record[MAX_RECORDS];



//*****************************************************************************
static void _wait_til_done(const tx_data_t* tx)
{
	// Wait until the processing thread indicates that it is done.

	for (;;)
	{
		if (tx->done)
			break;

		gsc_time_sleep_ms(1);
	}
}



//*****************************************************************************
static void _wait_til_rx_reading(const args_t* args)
{
	const rx_data_t*	rx	= args->rx;
	const tx_data_t*	tx	= args->tx;

	// Wait until the Rx thread begins reading data.

	for (;;)
	{
		if ((rx->reading) || (tx->stop))
			break;

		gsc_time_sleep_ms(1);
	}
}



//*****************************************************************************
static void _wait_til_started(const tx_data_t* tx)
{
	// Wait until the processing thread indicates that it has started.

	for (;;)
	{
		if ((tx->started) || (tx->stop))
			break;

		gsc_time_sleep_ms(1);
	}
}



//*****************************************************************************
static void _wait_for_start(const tx_data_t* tx)
{
	// Wait until the controlling thread tells us to start working.

	for (;;)
	{
		if ((tx->start) || (tx->stop))
			break;

		gsc_time_sleep_ms(1);
	}
}



//*****************************************************************************
static void _get_file_name(const args_t* args, gsc_buf_man_t* bm, const char* extension)
{
	char			buf[64];
	rx_data_t*		rx			= args->rx;
	tx_data_t*		tx			= args->tx;
	bm_user_area_t*	user_area	= (void*) bm->user_area;

	strcpy(tx->name, "datast");

	if (args->context_info)
	{
		sprintf(buf,
				"_st%lu.%03lu",
				(unsigned long) rx->start_time.tv_sec,
				(unsigned long) rx->start_time.tv_nsec / 1000000);
		strcat(tx->name, buf);

		// Test Number ====================================
		sprintf(buf, "_t#%03d", args->tests);
		strcat(tx->name, buf);

		// File Number ====================================
		sprintf(buf, "_f#%03d", _file_number);
		strcat(tx->name, buf);

		// Timestamp ======================================
		sprintf(buf, "_ts%ld.%03ld",
				(long) user_area->timestamp.tv_sec,
				(long) user_area->timestamp.tv_nsec / 1000000);
		strcat(tx->name, buf);

		// Channel Quantity ===============================
		sprintf(buf, "_cq%02d", (int) tx->chan_qty);
		strcat(tx->name, buf);

		// Sample Rate ====================================
		sprintf(buf, "_sr%.3f", (float) rx->fsamp);
		strcat(tx->name, buf);
	}

	strcat(tx->name, ".");
	strcat(tx->name, extension);
}



//*****************************************************************************
static void _get_file_pointer(const args_t* args, gsc_buf_man_t* bm, const char* extension)
{
	tx_data_t*	tx	= args->tx;

	if ((_file_number < 0) || ((tx->file_samples) && (_file_samples >= tx->file_samples)))
	{
		if (tx->file)
		{
			fclose(tx->file);
			tx->file	= NULL;
		}

		_file_number++;
		_file_samples	= 0;
		_get_file_name(args, bm, extension);

		tx->file	= fopen(tx->name, "w+b");

		if (tx->file == NULL)
		{
			tx->errs	= 1;
			sprintf(tx->err_buf, "File createion failure: %s", tx->name);
		}
	}
}



//*****************************************************************************
static void _write_file_bin(const args_t* args, gsc_buf_man_t* bm)
{
	int			ret;
	int			samples;
	tx_data_t*	tx		= args->tx;

	for (;;)	// A convenience loop.
	{
		_get_file_pointer(args, bm, "bin");

		if (tx->errs)
			break;

		samples	= bm->count / 4;

		ret	= fwrite((char*) bm->buffer + bm->offset, bm->count, 1, tx->file);

		if (ret < 0)
		{
			bm->count	= 0;
			tx->errs	= 1;
			sprintf(tx->err_buf, "Tx binary write failure: %d", (int) ret);
		}
		else
		{
			tx->total_bytes	+= bm->count;
			bm->count		= 0;
			bm->offset		= 0;
			_file_samples	+= samples;
		}

		break;
	}
}



//*****************************************************************************
static void _write_file_text(const args_t* args, gsc_buf_man_t* bm)
{
	char		buf[32];
	u32			chan		= 0;
	int			i;
	int			len;
	char*		prefix;
	u8*			ptr;
	int			remainder;
	int			ret			= 0;
	int			samples;
	u32*		src;
	tx_data_t*	tx			= args->tx;
	u32			value;

	for (;;)	// A convenience loop.
	{
		_get_file_pointer(args, bm, "txt");

		if (tx->errs)
			break;

		samples		= bm->count / 4;
		remainder	= bm->count % 4;
		src			= (void*) ((char*) bm->buffer + bm->offset);

		for (i = 0; i < samples; i++, src++)
		{
			value	= src[0];
			chan	= TAG_DECODE(value) ? 0 : chan + 1;

			if (args->tx_chan_tag == CHAN_TAG_EXCLUDE)
				value	&= VALUE_MASK;
			else if (args->tx_chan_tag == CHAN_TAG_ONLY)
				value	&= TAG_MASK;

			if (i == 0)
				prefix	= "";
			else
				prefix	= chan ? "  " : "\r\n";

			if (args->tx_decimal)
				sprintf(buf, "%s% 9ld", prefix, (long) value);
			else
				sprintf(buf, "%s%08lX", prefix, (long) value);

			len	= strlen(buf);
			ret	= fwrite(buf, len, 1, tx->file);

			if (ret < 0)
			{
				bm->count	= 0;
				tx->errs	= 1;
				sprintf(tx->err_buf, "Tx text write failure: %d", (int) ret);
				break;
			}
		}

		if (tx->errs)
			break;

		if (remainder)
		{
			buf[0]	= 0;
			ptr		= (u8*) src;

			if (samples)
				strcat(buf, "  ");

			sprintf(buf + strlen(buf), "%02lX", (long) (ptr[0] & 0xFF));

			if (remainder >= 2)
			{
				strcat(buf, "  ");
				sprintf(buf + strlen(buf), "%02lX", (long) (ptr[1] & 0xFF));
			}

			if (remainder >= 3)
			{
				strcat(buf, "  ");
				sprintf(buf + strlen(buf), "%02lX", (long) (ptr[2] & 0xFF));
			}

			strcat(buf, "\r\n");
			len	= strlen(buf);
			ret	= fwrite(buf, len, 1, tx->file);

			if (ret < 0)
			{
				bm->count	= 0;
				tx->errs	= 1;
				sprintf(tx->err_buf, "Tx text write failure: %d", (int) ret);
			}
		}

		if (ret >= 0)
		{
			tx->total_bytes	+= bm->count;
			bm->count		= 0;
			bm->offset		= 0;
			_file_samples	+= samples;
		}

		break;
	}
}



//*****************************************************************************
static void _validate_data(const gsc_buf_man_t* bm)
{
	long	chan_0_tags		= 0;
	long	err_values		= 0;
	long	l;
	long	non_zero_data	= 0;
	u32*	ptr				= bm->buffer;
	long	samples			= bm->count / 4;

	for (l = 0; l < samples; l++)
	{
		// D0-D15: data (16-bits)
		// Check for non-zero data values.

		if (ptr[l] & VALUE_MASK)
			non_zero_data++;

		// D16: channel 0 tag (1-bit)
		// Count the channel 0 tags.

		if (ptr[l] & TAG_MASK)
			chan_0_tags++;

		// D17-D31: reserved (15-bits)
		// Check for invalid values.

		if (ptr[l] & UNUSED_MASK)
			err_values++;
	}

	// Record the results.

	_record[_records].bm_count		= bm->count;
	_record[_records].err_values	= err_values;
	_record[_records].chan_0_tags	= chan_0_tags;
	_record[_records].non_zero_data	= non_zero_data;

	_records++;
}



//*****************************************************************************
static void _tx_process(const args_t* args)
{
	gsc_buf_man_t	bm;
	int				eof;
	int				errs	= 0;
	tx_data_t*		tx		= args->tx;

	memset(&bm, 0, sizeof(gsc_buf_man_t));
	_file_samples	= 0;

	if (sizeof(bm_user_area_t) > BUF_MAN_USER_AREA_SIZE)
	{
		errs		= 1;
		bm.eof		= 1;
		tx->errs	= 1;
		strcpy(tx->err_buf, "sizeof(xxx) > BUF_MAN_USER_AREA_SIZE");
	}

	if (errs == 0)
		_wait_til_rx_reading(args);

	tx->total_ms	= gsc_time_delta_ms();

	for (; errs == 0;)
	{
		// Request an data buffer so we can process it.
		errs	= gsc_buf_man_request_data(&bm);

		if (tx->stop)
		{
			gsc_buf_man_release_buffer(&bm);
			break;
		}

		// Perform validation.

		if (errs)
		{
			tx->errs	= 1;
			strcpy(tx->err_buf, "Tx data request: failed");
			gsc_buf_man_release_buffer(&bm);
			break;
		}

		if ((bm.buffer == NULL) && (bm.eof == 0))
		{
			tx->errs	= 1;
			strcpy(tx->err_buf, "Tx data request: NULL buffer");
			gsc_buf_man_release_buffer(&bm);
			break;
		}

		if ((bm.size == 0) && (bm.eof == 0))
		{
			tx->errs	= 1;
			strcpy(tx->err_buf, "Tx data request: zero sized buffer");
			gsc_buf_man_release_buffer(&bm);
			break;
		}

		if ((args->tx_validate) && (_records < MAX_RECORDS))
			_validate_data(&bm);

		switch (args->tx_option)
		{
			default:

				tx->errs	= 1;
				bm.count	= 0;
				sprintf(tx->err_buf, "Tx option invalid: %d", (int) args->tx_option);
				break;

			case TX_OPTION_BIT_BUCKET:

				tx->total_bytes	+= bm.count;
				bm.count		= 0;
				break;

			case TX_OPTION_WRITE_FILE_BIN:

				_write_file_bin(args, &bm);
				break;

			case TX_OPTION_WRITE_FILE_TEXT:

				_write_file_text(args, &bm);
				break;
		}

		eof		= bm.eof;
		bm.eof	= 0;
		errs	= gsc_buf_man_release_buffer(&bm);

		if ((tx->errs == 0) && (errs))
		{
			tx->errs	= 1;
			strcpy(tx->err_buf, "Tx buffer release failure");
		}

		if ((eof) || (tx->errs) || (tx->stop))
			break;
	}

	tx->total_ms	= gsc_time_delta_ms() - tx->total_ms;
}



//*****************************************************************************
static int _tx_thread(void* arg)
{
	const args_t*	args	= arg;
	tx_data_t*		tx		= args->tx;

	// Tell the controlling code that this thread has started.
	tx->started	= 1;

	// Wait till we're told to start.
	_wait_for_start(tx);

	// Perform the expected activity.
	_tx_process(args);

	// Tell the controlling code that we're done.
	tx->done	= 1;
	return(0);
}



//*****************************************************************************
int tx_start(const args_t* args)
{
	int				errs	= 0;
	const char*		ptr		= NULL;
	tx_data_t*		tx		= args->tx;

	gsc_label("Tx Startup");
	_file_number	= -1;

	switch (args->tx_option)
	{
		default:

			errs	= 1;
			ptr		= "invalid option";
			break;

		case TX_OPTION_BIT_BUCKET:

			ptr		= "discarding data";
			break;

		case TX_OPTION_WRITE_FILE_BIN:

			ptr	= "writing to binary file";
			break;

		case TX_OPTION_WRITE_FILE_TEXT:

			ptr	= "writing to text file";
			break;
	}

	if (errs == 0)
	{
		errs	= ai64ssa_query(args->fd, -1, 0, AI64SSA_QUERY_CHANNEL_QTY, &tx->chan_qty);

		if (errs)
			ptr	= "Channel count query";
	}

	if (errs == 0)
	{
		errs	= os_thread_create(&tx->thread, "Tx Thread", _tx_thread, (void*) args);

		if (errs)
			ptr	= "creating Tx thread";
	}

	if (errs)
	{
		printf("FAIL <---  (%s)\n", ptr);
	}
	else
	{
		_wait_til_started(tx);
		printf("PASS  (%s)\n", ptr);
	}

	return(errs);
}



//*****************************************************************************
int tx_stop(const args_t* args)
{
	int			errs	= 0;
	tx_data_t*	tx		= args->tx;

	// STOP ===============================================
	gsc_label("Tx Stop");
	tx->stop	= 1;

	if (tx->started)
		_wait_til_done(tx);

	if (tx->file)
	{
		fclose(tx->file);
		tx->file	= NULL;
	}

	os_thread_destroy(&tx->thread);
	printf("Done\n");

	gsc_label_level_inc();

	// STATUS =============================================
	gsc_label("Status");

	if (tx->errs == 0)
	{
		errs	= 0;
		printf("PASS\n");
	}
	else
	{
		errs	= 1;
		printf("FAIL <---");

		if (tx->err_buf[0])
			printf(" (%s)", tx->err_buf);

		printf("\n");
	}

	// THROUGHPUT =========================================
	gsc_label("Throughput");
	gsc_label_long_comma(tx->total_bytes / 4);
	printf(" samples, ");
	gsc_label_long_comma(tx->total_ms / 1000);
	printf(".%03ld Secs", (long) tx->total_ms % 1000);

	if (tx->total_ms)
	{
		printf(", ");
		gsc_label_long_comma(1000LL * tx->total_bytes / 4 / tx->total_ms);
		printf(" S/S");
	}

	printf("\n");

	gsc_label_level_dec();
	return(errs);
}



//*****************************************************************************
void tx_validation_results(void)
{
	long long	bm_count		= 0;
	long long	chan_0_tags		= 0;
	long long	err_values		= 0;
	int			i;
	long long	non_zero_data	= 0;
	float		ratio;
	long long	samples			= 0;

	printf("Tx Validation Results: %d records\n", _records);

	printf("  Record  Bytes       Samples     Error Vals  Chan 0 Tag  Non-Zero    NZD/C0T\n");
	printf("  ======  ==========  ==========  ==========  ==========  ==========  =======\n");

	for (i = 0; i < _records; i++)
	{
		printf("  %6d", i);
		printf("%12ld", (long) _record[i].bm_count);
		printf("%12ld", (long) _record[i].bm_count / 4);
		printf("%12ld", (long) _record[i].err_values);
		printf("%12ld", (long) _record[i].chan_0_tags);
		printf("%12ld", (long) _record[i].non_zero_data);

		if (_record[i].chan_0_tags)
		{
			ratio	= (float) _record[i].non_zero_data / _record[i].chan_0_tags;
			printf("  %7.4f", ratio);
		}
		else
		{
			printf("  inf");
		}

		printf("\n");

		bm_count		+= _record[i].bm_count;
		samples			+= _record[i].bm_count / 4;
		err_values		+= _record[i].err_values;
		chan_0_tags		+= _record[i].chan_0_tags;
		non_zero_data	+= _record[i].non_zero_data;
	}

	printf("  ======  ==========  ==========  ==========  ==========  ==========  =======\n");
	printf("  Totals");
	printf("%12lld", bm_count);
	printf("%12lld", samples);
	printf("%12lld", err_values);
	printf("%12lld", chan_0_tags);
	printf("%12lld", non_zero_data);

	if (chan_0_tags)
	{
		ratio	= (float) non_zero_data / chan_0_tags;
		printf("  %7.4f", ratio);
	}
	else
	{
		printf("  inf");
	}

	printf("\n");
}


