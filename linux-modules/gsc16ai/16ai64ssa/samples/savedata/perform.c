// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/savedata/perform.c $
// $Rev: 54968 $
// $Date: 2024-08-07 15:57:37 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



// macros *********************************************************************

#define	_1M					(1024L * 1024L)
#define	TAG_CH_0			0x10000
#define	TAG_DECODE(v)		((v) & 0x10000)
#define	TAG_MASK(v)			TAG_DECODE((v))
#define	VALUE_MASK(v)		((v) & 0xFFFF)
#define	UNUSED_MASK			0xFFFE0000



// variables ******************************************************************

static	u32	_buffer[_1M];



//*****************************************************************************
static int _channels(const args_t* args, s32* chans)
{
	s32		active;
	char	buf[64];
	int		errs	= 0;
	s32		first	= -1;
	s32		last	= -1;
	int		ret;
	s32		single	= -1;

	gsc_label("Input Channels");

	active	= -1;
	ret		= ai64ssa_ioctl(args->fd, AI64SSA_IOCTL_CHAN_ACTIVE,	&active	);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(args->fd, AI64SSA_IOCTL_CHAN_FIRST,	&first	);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(args->fd, AI64SSA_IOCTL_CHAN_LAST,	&last	);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(args->fd, AI64SSA_IOCTL_CHAN_SINGLE,	&single	);
	errs	+= ret ? 1 : 0;

	if (errs == 0)
	{
		switch (active)
		{
			default:

				errs++;
				printf("FAIL <---  (invalid ACTIVE selection: %ld)\n", (long) active);
				break;

			case AI64SSA_CHAN_ACTIVE_SINGLE:

				chans[0]	= 1;
				sprintf(buf, "# %ld", (long) single);
				break;

			case AI64SSA_CHAN_ACTIVE_0_1:

				chans[0]	= 2;
				strcpy(buf, "#s 0-1");
				break;

			case AI64SSA_CHAN_ACTIVE_0_3:

				chans[0]	= 4;
				strcpy(buf, "#s 0-3");
				break;

			case AI64SSA_CHAN_ACTIVE_0_7:

				chans[0]	= 8;
				strcpy(buf, "#s 0-7");
				break;

			case AI64SSA_CHAN_ACTIVE_0_15:

				chans[0]	= 16;
				strcpy(buf, "#s 0-15");
				break;

			case AI64SSA_CHAN_ACTIVE_0_31:

				chans[0]	= 32;
				strcpy(buf, "#s 0-31");
				break;

			case AI64SSA_CHAN_ACTIVE_0_63:

				chans[0]	= 64;
				strcpy(buf, "#s 0-63");
				break;

			case AI64SSA_CHAN_ACTIVE_RANGE:

				chans[0]	= last - first + 1;

				if (first == last)
					sprintf(buf, "# %ld", (long) first);
				else
					sprintf(buf, "#s %ld-%ld", (long) first, (long) last);

				break;
		}

		if (errs == 0)
		{
			printf(	"%ld Channel%s  (%s)\n",
					(long) chans[0],
					(chans[0] == 1) ? "" : "s",
					buf);
		}
	}

	return(errs);
}



//*****************************************************************************
// The analog input boards may initially provide invalid data for a period.
// This function is called to consume the leading invalid data. Some board
// models may need this while others won't. Any type of required settling
// delay should be documented in the board user manual.
static int _data_preread(const args_t* args)
{
	int	errs	= 0;
	int	get		= 1024;	// bytes
	int	got;
	int	i;
	int	j;
	u32	last	= 0;
	int	samples	= get / 4;
	u32	tag;
	int	tests	= 0;

	for (i = 1;; i++)
	{
		if (i >= 1000)
		{
			errs++;
			printf(" FAIL <--- (preread failed)\n");
			fflush(stdout);
			break;
		}

		memset(_buffer, 0, get);
		errs	= ai64ssa_ai_buf_clear(args->fd, -1, 0);

		if (errs)
		{
			printf(" FAIL <--- (preread clear buffer failed)\n");
			fflush(stdout);
			break;
		}

		got	= ai64ssa_read(args->fd, _buffer, get);
		got	= get;

		if (got < 0)
		{
			errs++;
			printf(" FAIL <--- (preread returned %d)\n", got);
			fflush(stdout);
			break;
		}
		else if (got != get)
		{
			errs++;
			printf(" FAIL <--- (preread requested ");
			gsc_label_long_comma(get);
			printf(" bytes, received ");
			gsc_label_long_comma(got);
			printf(")\n");
			fflush(stdout);
			break;
		}

		// Find the first channel 0 entry.

		for (j = 0; j < samples; j++)
		{
			tag	= TAG_DECODE(_buffer[j]);

			if (tag == TAG_CH_0)
				break;
		}

		if (i == 1)
		{
			// This is the first sample from the first channel.
			last	= _buffer[j];
		}
		else
		{
			if (_buffer[j] != last)
			{
				tests++;

				if (tests >= 5)	// Repeat this before accepting the results.
					break;
			}

			last	= _buffer[j];
		}
	}

	return(errs);
}



//*****************************************************************************
static int _read_data(const args_t* args)
{
	char	buf[80];
	int		errs	= 0;
	long	get		= sizeof(_buffer);
	int		got;
	int		repeat;

	for (repeat = 0; (errs == 0) && (repeat < args->repeat);)
	{
		repeat++;
		strcpy(buf, "Reading");

		if (args->repeat > 1)
		{
			strcat(buf, " #");
			gsc_label_long_comma_buf(repeat, buf + strlen(buf));
		}

		gsc_label(buf);

		memset(_buffer, 0, sizeof(_buffer));
		errs	= _data_preread(args);

		if (errs)
			break;

		errs	= ai64ssa_ai_buf_clear(args->fd, -1, 0);

		if (errs)
		{
			printf("FAIL <---  (setup errors)\n");
			break;
		}

		got	= ai64ssa_read(args->fd, _buffer, get);

		if (got < 0)
		{
			errs++;
			printf("FAIL <---  (read error: %d)\n", got);
			break;
		}

		if (got != get)
		{
			errs	= 1;
			printf(	"FAIL <---  (got %ld bytes, requested %ld)\n",
					(long) got,
					(long) get);
			break;
		}

		printf("PASS  (");
		gsc_label_long_comma(get / 4);
		printf(" samples)\n");
	}

	return(errs);
}



//*****************************************************************************
static int _save_eol(const char* name, FILE* file)
{
	int	errs	= 0;
	int	i;

	i	= fprintf(file, "\r\n");

	if (i != 2)
	{
		printf("FAIL <---  (fprintf() failure to %s)\n", name);
		errs	= 1;
	}

	return(errs);
}



//*****************************************************************************
static int _save_args(const args_t* args, const char* name, FILE* file)
{
	int			arg;
	int			errs	= 0;
	int			i;
	int			len;
	const char*	space	= "# ";

	for (arg = 0; arg < args->argc; arg++)
	{
		// space ==========================================
		len	= strlen(space);
		i	= fprintf(file, "%s", space);

		if (i != len)
		{
			printf("FAIL <---  (fprintf() failure to %s)\n", name);
			errs++;
		}

		// argument =======================================
		space	= " ";
		len		= strlen(args->argv[arg]);
		i		= fprintf(file, "%s", args->argv[arg]);

		if (i != len)
		{
			printf("FAIL <---  (fprintf() failure to %s)\n", name);
			errs++;
		}
	}

	errs	+= _save_eol(name, file);
	return(errs);
}



//*****************************************************************************
static int _save_value(const args_t* args, const char* name, FILE* file, u32 value)
{
	char	buf1[40];
	char	buf2[80];
	int		errs	= 0;
	int		i;
	int		len;
	int		mid 	= 0x008000;
	int		vi;
	int		width	= 8;

	if (args->chan_tag == CHAN_TAG_EXCLUDE)
		value	= VALUE_MASK(value);
	else if (args->chan_tag == CHAN_TAG_ONLY)
		value	= TAG_MASK(value);

	if (args->format == FORMAT_HEX)
	{
		sprintf(buf1, "%08lX", (long) value);
	}
	else if (args->format == FORMAT_DEC)
	{
		sprintf(buf1, "%ld", (long) value);
	}
	else if (args->format == FORMAT_DEC_0)
	{
		vi	= value - mid;
		sprintf(buf1, "%ld", (long) vi);
	}
	else
	{
		sprintf(buf1, "FORMAT %d", (int) args->format);
	}

	sprintf(buf2, "  %*s", width, buf1);
	len	= strlen(buf2);
	i	= fprintf(file, "%8s", buf2);

	if (i != len)
	{
		printf("FAIL <---  (fprintf() failure to %s)\n", name);
		errs	= 1;
	}

	return(errs);
}



//*****************************************************************************
static int _validate_data(int chans, u32 last, long index, u32 data)
{
	static	int	chan_tag	= 0;
	int			errs		= 0;
	int			expect;
	static	int	last_tag	= 0;

	if (data & UNUSED_MASK)
	{
		errs	= 1;
		printf(" FAIL <---  (Upper bits are invalid: [%ld] = 0x%08lX)\n", index, (long) data);
	}

	chan_tag	= TAG_DECODE(data) ? 0 : chan_tag + 1;

	if (chan_tag >= chans)
	{
		errs	= 1;
		printf(" FAIL <---  (Invalid Channel Tag: [%ld] = 0x%08lX)\n", index, (long) data);
	}

	if (last == 0xFFFFFFFF)
	{
		if (chan_tag != 0)
		{
			errs	= 1;
			printf(" FAIL <---  (First tag not zero: [%ld] = 0x%08lX)\n", index, (long) data);
		}
	}
	else
	{
		last_tag	= TAG_DECODE(last) ? 0 : last_tag + 1;
		expect		= (last_tag + 1) % chans;

		if (chan_tag != expect)
		{
			errs	= 1;
			printf(	" FAIL <---  (Lost Synchronozation: [%ld] = 0x%08lX, [%ld] = 0x%08lX)\n",
					index - 1,
					(long) last,
					index,
					(long) data);
		}
	}

	return(errs);
}



//*****************************************************************************
static int _save_data(const args_t* args, int chans, int errs)
{
	char		buf[64];
	FILE*		file;
	long		index;
	u32			last	= 0xFFFFFFFF;
	const char*	name	= "data.txt";
	long		samples	= sizeof(_buffer) / 4;
	int			tag_got;

	strcpy(buf, "Saving");

	if (args->force_save)
	{
		errs	= 0;
		strcat(buf, " (Forced)");
	}

	gsc_label(buf);

	if (args->ai_mode == AI64SSA_AI_MODE_DIFF)
		chans /= 2;

	if (chans <= 0)
		chans	= 1;

	for (;;)
	{
		if ((errs) && (args->force_save == 0))
		{
			printf("SKIPPED  (errors)\n");
			errs	= 0;
			break;
		}

		file	= fopen(name, "w+b");

		if (file == NULL)
		{
			printf("FAIL <---  (unable to create %s)\n", name);
			errs	= 1;
			break;
		}

		errs	+= _save_args(args, name, file);

		for (index = 0; index < samples; index++)
		{
			tag_got	= TAG_DECODE(_buffer[index]) ? 0 : 1;

			if ((index > 0) && (tag_got == 0))
				errs	+= _save_eol(name, file);

			errs	+= _save_value(args, name, file, _buffer[index]);
			errs	+= _validate_data(chans, last, index, _buffer[index]);
			last	= _buffer[index];
		}

		fclose(file);

		if (errs == 0)
			printf("PASS  (%s)\n", name);

		break;
	}

	return(errs);
}



//*****************************************************************************
static int _config(const args_t* args, double* voltage)
{
	s32	chan_active	= args->chan_active;
	s32	chan_first	= args->chan_first;
	s32	chan_last	= args->chan_last;
	s32	chan_single	= args->chan_single;
	s32	chans;
	int	err;
	int	errs		= 0;
	s32	fsamp;
	s32	range;
	int	tmp;

	gsc_label("Configure");
	printf("\n");
	gsc_label_level_inc();

	errs	+= ai64ssa_query(args->fd, -1, 1, AI64SSA_QUERY_CHANNEL_QTY, &chans);
	errs	+= ai64ssa_query(args->fd, -1, 1, AI64SSA_QUERY_CHAN_RANGE, &range);

	if (chan_active == AI64SSA_CHAN_ACTIVE_RANGE)
	{
		if (range == 0)
		{
			gsc_label(NULL);
			printf("Overwriting -acr command line option with -ac031. <---\n");
			chan_active	= AI64SSA_CHAN_ACTIVE_0_31;
		}
		else
		{
			if (chan_first > chans)
			{
				gsc_label(NULL);
				printf("Overwriting -fc# command line option with -fc31. <---\n");
				printf("First channel reduced to #31. <---\n");
				chan_first	= 31;
			}

			if (chan_last > chans)
			{
				gsc_label(NULL);
				printf("Overwriting -lc# command line option with -lc31. <---\n");
				chan_last	= 31;
			}

			if (chan_first > chan_last)
			{
				gsc_label(NULL);
				printf("First and last channel numbers swapped. <---\n");
				tmp			= chan_first;
				chan_first	= chan_last;
				chan_last	= tmp;
			}
		}
	}

	if (chan_active == AI64SSA_CHAN_ACTIVE_SINGLE)
	{
		if (chan_single > chans)
		{
			gsc_label("Single Channel Option");
			printf("Overwriting -sc# command line option with -sc31. <---\n");
			chan_single	= 31;
		}
	}

	if (chan_active == AI64SSA_CHAN_ACTIVE_0_63)
	{
		if (chans < 64)
		{
			gsc_label("Active Channel Selection");
			printf("Overwriting -ac063 command line option with -ac031. <---\n");
			chan_active	= AI64SSA_CHAN_ACTIVE_0_31;
		}
	}

	switch (chan_active)
	{
		case AI64SSA_CHAN_ACTIVE_RANGE:

			if (range)
			{
				fsamp	= 200000 / (chan_last - chan_first + 1);
				break;
			}

		default:
		case AI64SSA_CHAN_ACTIVE_0_63:		fsamp	= 5000;		break;
		case AI64SSA_CHAN_ACTIVE_0_31:		fsamp	= 10000;	break;
		case AI64SSA_CHAN_ACTIVE_0_15:		fsamp	= 20000;	break;
		case AI64SSA_CHAN_ACTIVE_0_7:		fsamp	= 40000;	break;
		case AI64SSA_CHAN_ACTIVE_0_3:		fsamp	= 80000;	break;
		case AI64SSA_CHAN_ACTIVE_0_1:		fsamp	= 80000;	break;
		case AI64SSA_CHAN_ACTIVE_SINGLE:	fsamp	= 160000;	break;
	}

	gsc_label("Configuration");
	err		= ai64ssa_config_ai			(args->fd, -1, 0, fsamp);
	errs	+= err;
	printf("%s\n", err ? "FAIL <---" : "PASS");

	errs	+= ai64ssa_rx_io_mode		(args->fd, -1, 1, args->io_mode, NULL);
	errs	+= ai64ssa_chan_active		(args->fd, -1, 1, chan_active, NULL);
	errs	+= ai64ssa_chan_single		(args->fd, -1, 1, chan_single, NULL);
	errs	+= ai64ssa_chan_first		(args->fd, -1, 1, chan_first, NULL);
	errs	+= ai64ssa_chan_last		(args->fd, -1, 1, chan_last, NULL);
	errs	+= ai64ssa_rx_io_timeout	(args->fd, -1, 1, 60, NULL);

	errs	+= ai64ssa_ai_range			(args->fd, -1, 1, args->v_range, NULL);

	errs	+= ai64ssa_ai_mode			(args->fd, -1, 1, args->ai_mode, NULL);
	errs	+= ai64ssa_autocal			(args->fd, -1, 1);

	errs	+= ai64ssa_rx_io_overflow	(args->fd, -1, 1, AI64SSA_IO_OVERFLOW_IGNORE, NULL);
	errs	+= ai64ssa_rx_io_underflow	(args->fd, -1, 1, AI64SSA_IO_UNDERFLOW_IGNORE, NULL);
	errs	+= ai64ssa_ai_buf_clear		(args->fd, -1, 1);
	errs	+= ai64ssa_ai_buf_overflow	(args->fd, -1, 1, AI64SSA_AI_BUF_OVERFLOW_CLEAR, NULL);
	errs	+= ai64ssa_ai_buf_underflow	(args->fd, -1, 1, AI64SSA_AI_BUF_UNDERFLOW_CLEAR, NULL);
	errs	+= ai64ssa_rx_io_timeout	(args->fd, -1, 1, 300, NULL);

	gsc_label_level_dec();
	return(errs);
}



//*****************************************************************************
int perform_tests(const args_t* args)
{
	s32		chans	= 32;	// assume the maximum
	int		errs	= 0;
	double	voltage	= 10.0;

	gsc_label("Capture & Save");
	printf("\n");
	gsc_label_level_inc();

	errs	+= _config					(args, &voltage);
	errs	+= _channels				(args, &chans);
	errs	+= ai64ssa_ai_buf_clear		(args->fd, -1, 1);
	errs	+= _read_data				(args);
	errs	+= ai64ssa_ai_buf_overflow	(args->fd, -1, 1, AI64SSA_AI_BUF_OVERFLOW_CHECK, NULL);
	errs	+= ai64ssa_ai_buf_underflow	(args->fd, -1, 1, AI64SSA_AI_BUF_UNDERFLOW_CHECK, NULL);
	errs	+= _save_data				(args, chans, errs);

	gsc_label_level_dec();
	return(errs);
}



