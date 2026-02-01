// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/vrange/perform.c $
// $Rev: 53551 $
// $Date: 2023-08-07 14:24:24 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



// macros *********************************************************************

#define	_1M				(1L * 1024 * 1024)



// variables ******************************************************************

static s32	_rx_data[_1M];

static struct
{
	s32	min;
	s32	max;
	s32	qty;
} _chan_data[64];

static const struct
{
	s32			value;
	const char*	name;
	s32			range_multiplier;
} _aim_list[]	=
{
	// value				name						range_multiplier
	{ AI64SSA_AI_MODE_SINGLE,	"Single Ended",			1	},
	{ AI64SSA_AI_MODE_PS_DIFF,	"Pseudo Differential",	2	},
	{ AI64SSA_AI_MODE_DIFF,		"Full Differential",	2	},
	{ AI64SSA_AI_MODE_ZERO,		"Vzero",				1	},
	{ AI64SSA_AI_MODE_VREF,		"Vref",					1	},
	{ -1, NULL }
};

static const struct
{
	s32			value;
	const char*	name;
	float		range;
	float		offset;
} _range_list[]	=
{
	{ AI64SSA_AI_RANGE_2_5V,	"+-2.5 Volts",	5.0,	2.5		},
	{ AI64SSA_AI_RANGE_5V,		"+-5.0 Volts",	10.0,	5.0		},
	{ AI64SSA_AI_RANGE_10V,		"+-10 VOlts",	20.0,	10.0	},
	{ AI64SSA_AI_RANGE_0_5V,	"0-5 Volts",	5.0,	0.0		},
	{ AI64SSA_AI_RANGE_0_10V,	"0-10 Volts",	10.0,	0.0		},
	{ -1, NULL }
};



//*****************************************************************************
static int _read_data(const args_t* args)
{
	int	bytes	= sizeof(_rx_data);
	int	errs	= 0;
	int	ret;

	gsc_label("Reading");
	ret	= ai64ssa_read(args->fd, _rx_data, bytes);

	if (ret < 0)
	{
		errs++;
		printf("FAIL <---  (ai64ssa_read() error, returned %d)\n", ret);
	}
	else if (ret != bytes)
	{
		errs++;
		printf("FAIL <---  (requested ");
		gsc_label_long_comma(bytes);
		printf(" bytes, received ");
		gsc_label_long_comma(ret);
		printf(")\n");
	}
	else
	{
		printf("PASS  (read ");
		gsc_label_long_comma(ret / 4);
		printf(" samples)\n");
	}

	return(errs);
}



//*****************************************************************************
static int _examine_data(const args_t* args, int errs)
{
	int	chan	= 0;
	int	i;
	s32	val;

	gsc_label("Examing Data");

	if (errs)
	{
		errs	= 0;
		printf("ABORTING due to errors\n");
	}
	else
	{
		for (i = 0; i < _1M; i++, chan++)
		{
			if (_rx_data[i] & 0x10000)
				chan	= 0;

			if (chan >= 64)
			{
				errs++;
				printf("FAIL <---  (missing channel tag: index ");
				gsc_label_long_comma(i);
				printf("\n");
				break;
			}

			val	= _rx_data[i] & 0xFFFF;

			if (_chan_data[chan].qty == 0)
			{
				_chan_data[chan].min	= val;
				_chan_data[chan].max	= val;
			}
			else if (_chan_data[chan].min > val)
			{
				_chan_data[chan].min	= val;
			}
			else  if (_chan_data[chan].max < val)
			{
				_chan_data[chan].max	= val;
			}

			_chan_data[chan].qty++;
		}

		if (errs == 0)
			printf("Done\n");
	}

	return(errs);
}



//*****************************************************************************
static int _report_data(const args_t* args, s32 chans, int delta, int errs)
{
	char	buf[64];
	int		chan		= 0;
	int		i;
	int		processed	= 0;

	gsc_label("Results");

	if (errs)
	{
		errs	= 0;
		printf("ABORTING due to errors\n");
	}
	else
	{
		printf("\n");
		gsc_label_level_inc();
		chans	/= delta;

		for (i = 0; i < chans; i++, chan += delta)
		{
			if ((args->ai_chan >= 0) && (args->ai_chan != chan))
				continue;

			processed	= 1;
			sprintf(buf, "Channel %d", chan);
			gsc_label(buf);

			printf(	"0x%04lX - 0x%04lX, %ld samples\n",
					(long) _chan_data[i].min,
					(long) _chan_data[i].max,
					(long) _chan_data[i].qty);
		}

		if (processed == 0)
		{
			sprintf(buf, "Channel %d", args->ai_chan);
			gsc_label(buf);
			printf("SKIPPED  (no data available)\n");
		}

		gsc_label_level_dec();
	}

	return(errs);
}



//*****************************************************************************
static int _ascii_graph_data(const args_t* args, int aim, int range, int delta)
{
	char	buf[64];
	int		chan	= 0;
	int		errs	= 0;
	int		i;
	int		processed	= 0;
	s32		val;
	float	volts;
	int		width;

	gsc_label("ASCII Graph");

	if ((args->ai_chan < 0) || (args->ai_chan > 63))
	{
		errs++;
		printf("FAIL <---  (invalid channel selection: %d\n", args->ai_chan);
	}
	else
	{
		printf("\n");
		gsc_label_level_inc();

		for (i = 0; i < _1M; i++, chan += delta)
		{
			if (_rx_data[i] & 0x10000)
				chan	= 0;

			if ((args->ai_chan >= 0) && (args->ai_chan != chan))
				continue;

			processed	= 1;
			val			= _rx_data[i] & 0xFFFF;
			volts		= (float) val / 0xFFFF
						* _range_list[range].range
						- _range_list[range].offset;
			volts		*= _aim_list[aim].range_multiplier;
			width		= (int) floor((float) args->ascii_graph * val / 0xFFFF + 0.5);

			sprintf(buf, "Value %d", i);
			gsc_label(buf);
			printf("0x%04lX:", (long) val);
			printf(	"%*s*%*s", width, "", args->ascii_graph - width, "");
			printf(	": %7.3f Volts\n", volts);
		}

		if (processed == 0)
		{
			gsc_label("Graph Data");
			printf("SKIPPED  (no data available for channel %d)\n", args->ai_chan);
		}

		gsc_label_level_dec();
	}

	return(errs);
}



//*****************************************************************************
static int _save_data(void)
{
	int			errs	= 0;
	FILE*		file;
	int			i;
	long		l;
	const char*	name	= "vrange.txt";

	gsc_label("Saving");

	for (;;)
	{
		file	= fopen(name, "w+b");

		if (file == NULL)
		{
			printf("FAIL <---  (unable to create %s)\n", name);
			errs	= 1;
			break;
		}

		for (l = 0; l < _1M; l++)
		{
			if ((l) && (_rx_data[l] & 0x10000))
			{
				i	= fprintf(file, "\r\n");

				if (i != 2)
				{
					printf("FAIL <---  (fprintf() failure to %s)\n", name);;
					errs	= 1;
					break;
				}
			}

			i	= fprintf(file, "  %08lX", (long) _rx_data[l]);

			if (i != 10)
			{
				printf("FAIL <---  (fprintf() failure to %s)\n", name);
				errs	= 1;
				break;
			}
		}

		fclose(file);

		if (errs == 0)
			printf("PASS  (%s)\n", name);

		break;
	}

	return(errs);
}



//*****************************************************************************
static int _reg_list(const args_t* args)
{
	int	errs;

	gsc_label("Register Listing");
	printf("\n\n");
	errs	= ai64ssa_reg_list(args->fd, 1);
	printf("\n");
	return(errs);
}



//*****************************************************************************
static int _run_test(
	const args_t*	args,
	s32				chans,
	s32				fsamp,
	int				config,
	int				aim,
	int				range)
{
	char	buf[64];
	int		delta	= (_aim_list[aim].value == AI64SSA_AI_MODE_DIFF) ? 2 : 1;
	int		errs	= 0;
	long	ms		= 1000;

	sprintf(buf, "Configuration %d", config);
	gsc_label(buf);
	printf("%s, %s\n", _aim_list[aim].name, _range_list[range].name);
	gsc_label_level_inc();

	memset(_rx_data, 0, sizeof(_rx_data));
	memset(_chan_data, 0, sizeof(_chan_data));

	errs	+= ai64ssa_config_ai(args->fd, -1, args->verbose, fsamp);
	errs	+= ai64ssa_ai_mode	(args->fd, -1, 1, _aim_list[aim].value, NULL);
	errs	+= ai64ssa_ai_range	(args->fd, -1, 1, _range_list[range].value, NULL);
	errs	+= ai64ssa_autocal	(args->fd, -1, 1);

	gsc_label("Settling Delay");
	gsc_time_sleep_ms(ms);
	printf("Done  (%ld ms)\n", ms);

	errs	+= ai64ssa_ai_buf_clear	(args->fd, -1, 1);
	errs	+= _read_data(args);
	errs	+= _examine_data(args, errs);
	errs	+= _report_data(args, chans, delta, errs);

	if (args->ascii_graph)
		errs	+= _ascii_graph_data(args, aim, range, delta);

	if (args->save)
		errs	+= _save_data();

	if (args->reg_list)
		errs	+= _reg_list(args);

	gsc_label_level_dec();
	return(errs);
}



//*****************************************************************************
int perform_tests(const args_t* args)
{
	s32	aim;
	s32	chans	= 64;
	int	config	= 0;
	int	errs	= 0;
	s32	fsamp	= 10000;
	s32	range;

	gsc_label("Channel Count");
	errs	+= ai64ssa_query(args->fd, -1, 0, AI64SSA_QUERY_CHANNEL_QTY, &chans);
	printf("%ld\n", (long) chans);

	gsc_label("Sample Rate");
	gsc_label_long_comma(fsamp);
	printf(" S/S\n");

	for (aim = 0; _aim_list[aim].value >= 0; aim++)
	{

		for (range = 0; _range_list[range].value >= 0; range++, config++)
		{
			if ((args->ai_mode >= 0) && (args->ai_mode != _aim_list[aim].value))
				continue;

			if ((args->ai_voltage >= 0) && (args->ai_voltage != _range_list[range].value))
				continue;

			errs	+= _run_test(args, chans, fsamp, config, aim, range);
		}
	}

	return(errs);
}


