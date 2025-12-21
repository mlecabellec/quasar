// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_fsamp_ai.c $
// $Rev: 53557 $
// $Date: 2023-08-07 14:30:23 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Utilities: source file

#include "main.h"



// macros *********************************************************************

#define	ABS(v)					(((v) < 0) ? (-(v)) : (v))
#define	LIMIT_MAX(v,max)		(((v) > (max)) ? (max) : (v))
#define	LIMIT_MIN(v,min)		(((v) < (min)) ? (min) : (v))
#define	LIMIT_RANGE(v,min,max)	(LIMIT_MIN((LIMIT_MAX((v),(max))),(min)))



// data types *****************************************************************

typedef struct
{
	s32		fsamp_want;
	double	fsamp_got;

	double	delta;
	s32		enable_a;
	s32		enable_b;
	s32		master;
	s32		fsamp_max;
	s32		fsamp_min;
	s32		nrate_a;
	s32		nrate_b;
	s32		nrate_max;
	s32		nrate_min;
	s32		src;
	s32		src_b;
} fsamp_t;



//*****************************************************************************
static int _channels(int fd, int verbose, s32* first, s32* last)
{
	s32	active	= -1;
	int	errs	= 0;
	s32	qty;
	int	ret;
	s32	single	= -1;

	first[0]	= -1;
	last[0]		= -1;

	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_CHANNEL_QTY,		&qty);
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_CHAN_ACTIVE,	&active	);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_CHAN_FIRST,	first	);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_CHAN_LAST,	last	);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_CHAN_SINGLE,	&single	);
	errs	+= ret ? 1 : 0;

	switch (active)
	{
		default:

			if ((errs == 0) && (verbose))
				printf("FAIL <---  (invalid ACTIVE selection: %ld)\n", (long) active);

			errs++;
			break;

		case AI64SSA_CHAN_ACTIVE_0_1:		first[0]	= 0;		last[0]	= 1;		break;
		case AI64SSA_CHAN_ACTIVE_0_3:		first[0]	= 0;		last[0]	= 3;		break;
		case AI64SSA_CHAN_ACTIVE_0_7:		first[0]	= 0;		last[0]	= 7;		break;
		case AI64SSA_CHAN_ACTIVE_0_15:		first[0]	= 0;		last[0]	= 15;		break;
		case AI64SSA_CHAN_ACTIVE_0_31:		first[0]	= 0;		last[0]	= 31;		break;
		case AI64SSA_CHAN_ACTIVE_0_63:		first[0]	= 0;		last[0]	= 63;		break;
		case AI64SSA_CHAN_ACTIVE_SINGLE:	first[0]	= single;	last[0]	= single;	break;
		case AI64SSA_CHAN_ACTIVE_RANGE:													break;
	}

	return(errs);
}



//*****************************************************************************
static int _rag_disabled(s32* sps, char* buf)
{
	s32	fsamp	= 0;

	strcpy(buf, "Rate-A Generator Disabled");

	if (sps)
		sps[0]	= fsamp;

	return(0);
}



//*****************************************************************************
static int _rbg_disabled(s32* sps, char* buf)
{
	s32	fsamp	= 0;

	strcpy(buf, "Rate-B Generator Disabled");

	if (sps)
		sps[0]	= fsamp;

	return(0);
}



//*****************************************************************************
static int _clk_rag_mc(int fd, s32* sps, char* buf)
{
	s32	a_enable	= -1;
	s32	a_nrate		= -1;
	int	errs		= 0;
	s32	fsamp		= 0;
	s32	max;
	s32	ref;
	int	ret;

	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_MASTER_CLOCK,	&ref);
	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_FSAMP_MAX,		&max);
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RAG_ENABLE,	&a_enable);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RAG_NRATE,	&a_nrate);
	errs	+= ret ? 1 : 0;

	for (;;)	// A convenience loop.
	{
		if (a_enable == AI64SSA_GEN_ENABLE_NO)
		{
			errs	+= _rag_disabled(&fsamp, buf);
			break;
		}

		sprintf(buf, "Fref %ld, A Nrate %ld", (long) ref, (long) a_nrate);

		// Rate-A Generator calculations
		fsamp	= ref / a_nrate;

		if (fsamp > max)
			strcat(buf, "  <--- UNSTABLE");

		break;
	}

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



//*****************************************************************************
static int _clk_rbg_mc(int fd, s32* sps, char* buf)
{
	s32	b_enable	= -1;
	s32	b_nrate		= -1;
	int	errs		= 0;
	s32	fsamp		= 0;
	s32	max;
	s32	ref;
	int	ret;

	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_MASTER_CLOCK,	&ref);
	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_FSAMP_MAX,		&max);
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RBG_ENABLE,	&b_enable);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RBG_NRATE,	&b_nrate);
	errs	+= ret ? 1 : 0;

	for (;;)	// A convenience loop.
	{
		if (b_enable == AI64SSA_GEN_ENABLE_NO)
		{
			errs	+= _rbg_disabled(&fsamp, buf);
			break;
		}

		sprintf(buf, "Fref %ld, B Nrate %ld", (long) ref, (long) b_nrate);

		// Rate-A Generator calculations
		fsamp	= ref / b_nrate;

		if (fsamp > max)
			strcat(buf, "  <--- UNSTABLE");

		break;
	}

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



//*****************************************************************************
static int _clk_rbg_rag_mc(int fd, s32* sps, char* buf)
{
	s32	a_enable	= -1;
	s32	a_nrate		= -1;
	s32	b_enable	= -1;
	s32	b_nrate		= -1;
	int	errs		= 0;
	s32	fsamp		= 0;
	s32	max;
	s32	ref;
	int	ret;

	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_MASTER_CLOCK,	&ref);
	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_FSAMP_MAX,		&max);
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RAG_ENABLE,	&a_enable);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RAG_NRATE,	&a_nrate);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RBG_ENABLE,	&b_enable);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RBG_NRATE,	&b_nrate);
	errs	+= ret ? 1 : 0;

	for (;;)	// A convenience loop.
	{
		if (a_enable == AI64SSA_GEN_ENABLE_NO)
		{
			errs	+= _rag_disabled(&fsamp, buf);
			break;
		}

		if (b_enable == AI64SSA_GEN_ENABLE_NO)
		{
			errs	+= _rbg_disabled(&fsamp, buf);
			break;
		}

		sprintf(buf,
				"Fref %ld, A Nrate %ld, B Nrate %ld",
				(long) ref,
				(long) a_nrate,
				(long) b_nrate);

		// Rate-A/B Generator calculations
		fsamp	= ref / (a_nrate * b_nrate);

		if (fsamp > max)
			strcat(buf, "  <--- UNSTABLE");

		break;
	}

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



//*****************************************************************************
static int _clk_rbg(int fd, s32* sps, char* buf)
{
	int	errs;
	s32	fsamp	= 0;
	s32	rbg		= -1;
	int	ret;

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RBG_CLK_SRC, &rbg);
	errs	= ret ? 1 : 0;

	if (errs == 0)
	{
		switch (rbg)
		{
			default:

				sprintf(buf, "%d. INTERNAL ERROR: %ld", __LINE__, (long) rbg);
				break;

			case AI64SSA_RBG_CLK_SRC_MASTER:

				errs	+= _clk_rbg_mc(fd, &fsamp, buf);
				break;

			case AI64SSA_RBG_CLK_SRC_RAG:

				errs	+= _clk_rbg_rag_mc(fd, &fsamp, buf);
				break;
		}
	}

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



//*****************************************************************************
static int _clk_ext(s32* sps, char* buf)
{
	s32	fsamp	= 0;

	strcpy(buf, "Externally Clocked Sampling");

	if (sps)
		sps[0]	= fsamp;

	return(0);
}



//*****************************************************************************
static int _clk_bcr(s32* sps, char* buf)
{
	s32	fsamp	= 0;

	strcpy(buf, "Software Clocked Sampling");

	if (sps)
		sps[0]	= fsamp;

	return(0);
}



//*****************************************************************************
static int _burst_disable(int fd, s32* sps, char* buf)
{
	int	errs;
	s32	fsamp	= 0;
	int	ret;
	s32	src		= -1;

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_SAMP_CLK_SRC, &src);
	errs	= ret ? 1 : 0;

	if (errs == 0)
	{
		switch (src)
		{
			default:

				sprintf(buf, "%d. INTERNAL ERROR: %ld", __LINE__, (long) src);
				break;

			case AI64SSA_SAMP_CLK_SRC_RAG:

				errs	+= _clk_rag_mc(fd, &fsamp, buf);
				break;

			case AI64SSA_SAMP_CLK_SRC_RBG:

				errs	+= _clk_rbg(fd, &fsamp, buf);
				break;

			case AI64SSA_SAMP_CLK_SRC_EXT:

				errs	+= _clk_ext(&fsamp, buf);
				break;

			case AI64SSA_SAMP_CLK_SRC_BCR:

				errs	+= _clk_bcr(&fsamp, buf);
				break;
		}
	}

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



//*****************************************************************************
static int _burst_rbg_mc_clk_rag_mc(int fd, s32* sps, char* buf)
{
	s32			a_enable	= -1;
	s32			a_nrate		= -1;
	s32			b_enable	= -1;
	s32			b_nrate		= -1;
	float		b_tb;				// Burst: Time Burst
	long long	b_tb_ns;			// Burst: Time Burst Nano Seconds.
	long long	b_tt_ns;			// Burst: Time Total Nano Seconds.
	float		bps;				// Bursts Per Second
	int			errs		= 0;
	s32			fsamp		= 0;
	s32			max;
	long		qty;
	s32			ref;
	int			ret;
	long long	s_tb_ns;			// Sample: Time Burst Nano Seconds.
	float		s_ts;				// Sample: Time Scan
	long long	s_ts_ns;			// Sample: Time Scan Nano Seconds.
	s32			size		= -1;

	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_MASTER_CLOCK,	&ref);
	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_FSAMP_MAX,		&max);
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_BURST_SIZE,	&size);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RAG_ENABLE,	&a_enable);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RAG_NRATE,	&a_nrate);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RBG_ENABLE,	&b_enable);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RBG_NRATE,	&b_nrate);
	errs	+= ret ? 1 : 0;

	for (;;)	// A convenience loop.
	{
		if (a_enable == AI64SSA_GEN_ENABLE_NO)
		{
			fsamp	= 0;
			sprintf(buf, "Rate-A Generator disabled");
			break;
		}

		if (b_enable == AI64SSA_GEN_ENABLE_NO)
		{
			fsamp	= 0;
			sprintf(buf, "Rate-B Generator disabled");
			break;
		}

		sprintf(buf,
				"Fref %ld, A Nrate %ld, B Nrate %ld, Burst Size %ld",
				(long) ref,
				(long) a_nrate,
				(long) b_nrate,
				(long) size);

		if (size == 0)
		{
			fsamp	= ref / a_nrate;
			break;
		}

		// Sample clocking
		s_ts	= (float) a_nrate / ref;
		s_ts_ns	= (long long) (s_ts * 1000000000L + 0.5);
		s_tb_ns	= s_ts_ns * size;

		// Burst Clocking
		b_tb	= (float) b_nrate / ref;
		b_tb_ns	= (long long) (b_tb * 1000000000L + 0.5);
		qty		= (long) ((s_tb_ns + s_ts_ns - 1) / b_tb_ns);
		qty		= (qty < 1) ? 1 : qty;
		b_tt_ns	= b_tb_ns * qty;

		// Sample rate.
		bps		= 1.0 / (1.0 * b_tt_ns / 1000000000L);
		fsamp	= (s32) (bps * size);
		break;
	}

	if ((errs == 0) && (fsamp > max))
		strcat(buf, "  <--- UNSTABLE");

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



//*****************************************************************************
static int _burst_rbg_mc_clk_rbg_mc(s32* sps, char* buf)
{
	s32	fsamp	= 0;

	strcpy(buf, "Invalid Configuration");

	if (sps)
		sps[0]	= fsamp;

	return(0);
}



//*****************************************************************************
static int _burst_rbg_mc(int fd, s32* sps, char* buf)
{
	s32	adc		= -1;
	int	errs;
	s32	fsamp	= 0;
	int	ret;

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_SAMP_CLK_SRC, &adc);
	errs	= ret ? 1 : 0;

	if (errs == 0)
	{
		switch (adc)
		{
			default:

				sprintf(buf, "%d. INTERNAL ERROR: %ld", __LINE__, (long) adc);
				break;

			case AI64SSA_SAMP_CLK_SRC_BCR:

				errs	+= _clk_bcr(&fsamp, buf);
				break;

			case AI64SSA_SAMP_CLK_SRC_EXT:

				errs	+= _clk_ext(&fsamp, buf);
				break;

			case AI64SSA_SAMP_CLK_SRC_RAG:

				errs	+= _burst_rbg_mc_clk_rag_mc(fd, &fsamp, buf);
				break;

			case AI64SSA_SAMP_CLK_SRC_RBG:

				errs	+= _burst_rbg_mc_clk_rbg_mc(&fsamp, buf);
				break;
		}
	}

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



//*****************************************************************************
static int _burst_rbg_rag_mc_clk_rag_mc(int fd, s32* sps, char* buf)
{
	s32			a_enable	= -1;
	s32			a_nrate		= -1;
	s32			b_enable	= -1;
	s32			b_nrate		= -1;
	float		b_tb;				// Burst: Time Burst
	long long	b_tb_ns;			// Burst: Time Burst Nano Seconds.
	long long	b_tt_ns;			// Burst: Time Total Nano Seconds.
	float		bps;				// Bursts Per Second
	int			errs		= 0;
	s32			fsamp		= 0;
	s32			max;
	long		qty;
	s32			ref;
	int			ret;
	long long	s_tb_ns;			// Sample: Time Burst Nano Seconds.
	float		s_ts;				// Sample: Time Scan
	long long	s_ts_ns;			// Sample: Time Scan Nano Seconds.
	s32			size		= -1;

	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_MASTER_CLOCK,	&ref);
	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_FSAMP_MAX,		&max);
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_BURST_SIZE,	&size);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RAG_ENABLE,	&a_enable);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RAG_NRATE,	&a_nrate);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RBG_ENABLE,	&b_enable);
	errs	+= ret ? 1 : 0;
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RBG_NRATE,	&b_nrate);
	errs	+= ret ? 1 : 0;

	for (;;)	// A convenience loop.
	{
		if (a_enable == AI64SSA_GEN_ENABLE_NO)
		{
			fsamp	= 0;
			sprintf(buf, "Rate-A Generator disabled");
			break;
		}

		if (b_enable == AI64SSA_GEN_ENABLE_NO)
		{
			fsamp	= 0;
			sprintf(buf, "Rate-B Generator disabled");
			break;
		}

		sprintf(buf,
				"Fref %ld, A Nrate %ld, B Nrate %ld, Burst Size %ld",
				(long) ref,
				(long) a_nrate,
				(long) b_nrate,
				(long) size);

		if (size == 0)
		{
			fsamp	= ref / a_nrate;
			break;
		}

		// Sample clocking
		s_ts	= (float) a_nrate / ref;
		s_ts_ns	= (long long) (s_ts * 1000000000L + 0.5);
		s_tb_ns	= s_ts_ns * size;

		// Burst Clocking
		b_tb	= ((float) a_nrate * b_nrate) / ref;
		b_tb_ns	= (long long) (b_tb * 1000000000L + 0.5);
		qty		= (long) ((s_tb_ns + s_ts_ns - 1) / b_tb_ns);
		qty		= (qty < 1) ? 1 : qty;
		b_tt_ns	= b_tb_ns * qty;

		// Sample rate.
		bps		= 1.0 / (1.0 * b_tt_ns / 1000000000L);
		fsamp	= (s32) (bps * size);
		break;
	}

	if ((errs == 0) && (fsamp > max))
		strcat(buf, "  <--- UNSTABLE");

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



//*****************************************************************************
static int _burst_rbg_rag_mc_clk_rbg_rag_mc(s32* sps, char* buf)
{
	s32	fsamp	= 0;

	strcpy(buf, "Invalid Configuration");

	if (sps)
		sps[0]	= fsamp;

	return(0);
}



//*****************************************************************************
static int _burst_rbg_rag_mc(int fd, s32* sps, char* buf)
{
	int	errs;
	s32	fsamp	= 0;
	int	ret;
	s32	src		= -1;

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_SAMP_CLK_SRC, &src);
	errs	= ret ? 1 : 0;

	if (errs == 0)
	{
		switch (src)
		{
			default:

				sprintf(buf, "%d. INTERNAL ERROR: %ld", __LINE__, (long) src);
				break;

			case AI64SSA_SAMP_CLK_SRC_BCR:

				errs	+= _clk_bcr(&fsamp, buf);
				break;

			case AI64SSA_SAMP_CLK_SRC_EXT:

				errs	+= _clk_ext(&fsamp, buf);
				break;

			case AI64SSA_SAMP_CLK_SRC_RAG:

				errs	+= _burst_rbg_rag_mc_clk_rag_mc(fd, &fsamp, buf);
				break;

			case AI64SSA_SAMP_CLK_SRC_RBG:

				errs	+= _burst_rbg_rag_mc_clk_rbg_rag_mc(&fsamp, buf);
				break;
		}
	}

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



//*****************************************************************************
static int _burst_rbg(int fd, s32* sps, char* buf)
{
	int	errs;
	s32	fsamp	= 0;
	s32	rbg		= -1;
	int	ret;

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_RBG_CLK_SRC, &rbg);
	errs	= ret ? 1 : 0;

	if (errs == 0)
	{
		switch (rbg)
		{
			default:

				sprintf(buf, "%d. INTERNAL ERROR: %ld", __LINE__, (long) rbg);
				break;

			case AI64SSA_RBG_CLK_SRC_MASTER:

				errs	+= _burst_rbg_mc(fd, &fsamp, buf);
				break;

			case AI64SSA_RBG_CLK_SRC_RAG:

				errs	+= _burst_rbg_rag_mc(fd, &fsamp, buf);
				break;
		}
	}

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



//*****************************************************************************
static int _burst_ext(s32* sps, char* buf)
{
	s32	fsamp	= 0;

	strcpy(buf, "Externally Triggered Bursting");

	if (sps)
		sps[0]	= fsamp;

	return(0);
}



//*****************************************************************************
static int _burst_bcr(s32* sps, char* buf)
{
	s32	fsamp	= 0;

	strcpy(buf, "Software Triggered Bursting");

	if (sps)
		sps[0]	= fsamp;

	return(0);
}



//*****************************************************************************
static void _fsamp_compute_a(fsamp_t* data)
{
	float	delta;
	float	fsamp_got;
	float	fsamp_want	= data->fsamp_want;
	int		init		= 0;	// Have we performed the first run initialization?
	s32		master		= data->master;
	s32		nr;
	s32		nr_max;
	s32		nr_min;
	s32		nrate_a;
	s32		nrate_max	= data->nrate_max;
	s32		nrate_min	= data->nrate_min;

	data->src		= AI64SSA_SAMP_CLK_SRC_RAG;
	data->src_b		= AI64SSA_RBG_CLK_SRC_MASTER;
	data->enable_a	= AI64SSA_GEN_ENABLE_YES;
	data->enable_b	= AI64SSA_GEN_ENABLE_NO;

	nr		= master / fsamp_want;
	nr		= (nr < nrate_min) ? nrate_min
			: (nr > nrate_max) ? nrate_max : nr;

	nr_min	= nr - 5;
	nr_max	= nr + 5;
	nr_min	= (nr_min < nrate_min) ? nrate_min
			: (nr_min > nrate_max) ? nrate_max : nr_min;
	nr_max	= (nr_max < nrate_min) ? nrate_min
			: (nr_max > nrate_max) ? nrate_max : nr_max;

	for (nrate_a = nr_min; nrate_a <= nr_max; nrate_a++)
	{
		fsamp_got	= ((float) master) / nrate_a;
		delta		= fsamp_got - fsamp_want;

		if ((init == 0) || ((ABS(delta)) < (ABS(data->delta))))
		{
			init			= 1;
			data->delta		= delta;
			data->fsamp_got	= fsamp_got;
			data->nrate_a	= nrate_a;
		}
	}
}



//*****************************************************************************
static void _fsamp_compute_ab(fsamp_t* data)
{
	float	delta;
	float	fsamp_got;
	float	fsamp_want	= data->fsamp_want;
	int		init		= 0;	// Have we performed the first run initialization?
	s32		master		= data->master;
	s32		nr;
	s32		nr_max;
	s32		nr_min;
	s32		nrate_a;
	s32		nrate_b;
	s32		nrate_max	= data->nrate_max;
	s32		nrate_min	= data->nrate_min;

	data->src		= AI64SSA_SAMP_CLK_SRC_RBG;
	data->src_b		= AI64SSA_RBG_CLK_SRC_RAG;
	data->enable_a	= AI64SSA_GEN_ENABLE_YES;
	data->enable_b	= AI64SSA_GEN_ENABLE_YES;

	for (nrate_a = nrate_min; nrate_a <= nrate_max; nrate_a++)
	{
		nr		= master / fsamp_want / nrate_a;
		nr		= (nr < nrate_min) ? nrate_min
				: (nr > nrate_max) ? nrate_max : nr;

		nr_min	= nr - 5;
		nr_max	= nr + 5;
		nr_min	= (nr_min < nrate_min) ? nrate_min
				: (nr_min > nrate_max) ? nrate_max : nr_min;
		nr_max	= (nr_max < nrate_min) ? nrate_min
				: (nr_max > nrate_max) ? nrate_max : nr_max;

		for (nrate_b = nr_min; nrate_b <= nr_max; nrate_b++)
		{
			fsamp_got	= (float) master / ((float) nrate_b * nrate_a);
			delta		= fsamp_got - fsamp_want;

			if ((init == 0) || ((ABS(delta)) < (ABS(data->delta))))
			{
				init			= 1;
				data->delta		= delta;
				data->fsamp_got	= fsamp_got;
				data->nrate_a	= nrate_a;
				data->nrate_b	= nrate_b;
			}
		}
	}
}



/******************************************************************************
*
*	Function:	ai64ssa_fsamp_ai_report
*
*	Purpose:
*
*		Determine and report the sample rate for the specified channel.
*
*	Arguments:
*
*		fd		The handle to use to access the driver.
*
*		index	The index of the device to access. Ignore if < 0.
*
*		verbose	Work verbosely?
*
*		chan	The index of the channel of interest.
*
*		sps		Store the sample rate here, if non-NULL.
*
*	Returned:
*
*		>= 0	The number of errors encountered here.
*
******************************************************************************/

int ai64ssa_fsamp_ai_report(int fd, int index, int verbose, int chan, s32* sps)
{
	char	buf[1024];
	s32		burst		= -1;
	int		errs		= 0;
	s32		first;
	s32		fsamp		= 0;
	s32		last;
	s32		mode		= -1;
	s32		qty;
	int		ret;

	if (chan < 0)
		sprintf(buf, "Sample Rate");
	else
		sprintf(buf, "Channel %d", chan);

	if (verbose)
		gsc_label_index(buf, index);

	if (chan < 0)
		chan	= 0;

	for (;;)
	{
		ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_AI_MODE, &mode);
		errs	+= ret ? 1 : 0;
		errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_CHANNEL_QTY, &qty);

		if (errs)
			break;

		if ((chan < 0) || (chan > (qty - 1)))
		{
			errs++;

			if (verbose)
				printf("FAIL <---  (invalid channel index: %d)\n", chan);

			break;
		}

		errs	+= _channels(fd, verbose, &first, &last);

		if (errs)
			break;

		if ((chan < first) || (chan > last))
		{
			fsamp	= 0;

			if (verbose)
				printf("SKIPPED  (disabled)\n");

			break;
		}

		ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_BURST_SRC, &burst);
		errs	+= ret ? 1 : 0;

		if (errs)
			break;

		if ((mode == AI64SSA_AI_MODE_DIFF) && (chan % 2))
		{
			fsamp	= 0;

			if (verbose)
				printf("SKIPPED  (Low side of Full Differential channel %d.)\n", chan - 1);

			break;
		}

		switch (burst)
		{
			default:

				sprintf(buf, "%d. INTERNAL ERROR: %ld", __LINE__, (long) burst);
				break;

			case AI64SSA_BURST_SRC_DISABLE:

				errs	+= _burst_disable(fd, &fsamp, buf);
				break;

			case AI64SSA_BURST_SRC_RBG:

				errs	+= _burst_rbg(fd, &fsamp, buf);
				break;

			case AI64SSA_BURST_SRC_EXT:

				errs	+= _burst_ext(&fsamp, buf);
				break;

			case AI64SSA_BURST_SRC_BCR:

				errs	+= _burst_bcr(&fsamp, buf);
				break;
		}

		break;
	}

	if ((errs == 0) && (verbose))
	{
		gsc_label_long_comma((long) fsamp);
		printf(" S/S  (%s)\n", buf);
	}

	if (sps)
		sps[0]	= fsamp;

	return(errs);
}



/******************************************************************************
*
*	Function:	ai64ssa_fsamp_ai_report_all
*
*	Purpose:
*
*		Determine and report the sample rate for all active channels.
*
*	Arguments:
*
*		fd		The handle to use to access the driver.
*
*		index	The index of the device to access. Ignore if < 0.
*
*		verbose	Work verbosely?
*
*		fsamp	Report the overall sample rate here.
*
*	Returned:
*
*		>= 0	The number of errors encountered here.
*
******************************************************************************/

int ai64ssa_fsamp_ai_report_all(int fd, int index, int verbose, s32* fsamp)
{
	int	errs	= 0;
	s32	first	= 0;
	int	i;
	s32	last	= 0;
	s32	mode	= -1;
	int	ret;
	s32	sps;
	s32	total	= 0;

	if (verbose)
		gsc_label_index("Sample Rates", index);

	errs	+= _channels(fd, verbose, &first, &last);
	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_AI_MODE, &mode);
	errs	+= ret ? 1 : 0;

	if (errs == 0)
	{
		if (verbose)
		{
			if (mode == AI64SSA_AI_MODE_DIFF)
				printf("(Odd channels skipped per Full Differential operation.)");

			printf("\n");
			gsc_label_level_inc();
		}

		for (i = first; i <= last; i++)
		{
			if ((mode == AI64SSA_AI_MODE_DIFF) && (i % 2))
				continue;

			errs	+= ai64ssa_fsamp_ai_report(fd, index, verbose, i, &sps);
			total	+= sps;
		}

		if (verbose)
		{
			gsc_label("Overall Rate");
			gsc_label_long_comma(total);
			printf(" S/S\n");
			gsc_label_level_dec();
		}
	}

	if (fsamp)
		fsamp[0]	= total;

	return(errs);
}



//*****************************************************************************
int	ai64ssa_fsamp_ai_compute(
	int		fd,
	int		index,
	int		verbose,
	s32		fsamp,
	s32*	src,
	s32*	src_b,
	s32*	nrate_a,
	s32*	nrate_b,
	s32*	enable_a,
	s32*	enable_b,
	double*	rate)
{
	fsamp_t	data;
	fsamp_t	data_a;
	fsamp_t	data_ab;
	int		errs		= 0;

	if (verbose)
		gsc_label_index("AI Sample Rate", index);

	memset(&data, 0, sizeof(data));
	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_FSAMP_MAX, &data.fsamp_max);
	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_FSAMP_MIN, &data.fsamp_min);
	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_MASTER_CLOCK, &data.master);
	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_NRATE_MAX, &data.nrate_max);
	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_NRATE_MIN, &data.nrate_min);

	data.delta		= FLT_MAX;
	data.fsamp_want	= LIMIT_RANGE(fsamp, data.fsamp_min, data.fsamp_max);

	data_a	= data;
	data_ab	= data;

	_fsamp_compute_a(&data_a);
	_fsamp_compute_ab(&data_ab);

	if ((ABS(data_a.delta)) < (ABS(data_ab.delta)))
	{
		data_a.nrate_b	= data.nrate_max;
		data_a.enable_b	= AI64SSA_GEN_ENABLE_NO;
		data			= data_a;

		if (verbose)
		{
			printf("%s  (", errs ? "FAIL <---" : "PASS");
			gsc_label_long_comma(fsamp);
			printf(" S/S: ");
			printf("Fref ");
			gsc_label_long_comma((long) data.master);
			printf(", Nrate-A ");
			gsc_label_long_comma((long) data.nrate_a);
			printf(", Fsamp %.3f", data.fsamp_got);
			printf(")\n");
		}
	}
	else
	{
		data	= data_ab;

		if (verbose)
		{
			printf("%s  (", errs ? " FAIL <---" : "");
			gsc_label_long_comma(fsamp);
			printf(" S/S: ");
			printf("Fref ");
			gsc_label_long_comma((long) data.master);
			printf(", Nrate-A ");
			gsc_label_long_comma((long) data.nrate_a);
			printf(", Nrate-B ");
			gsc_label_long_comma((long) data.nrate_b);
			printf(", Fsamp %.3f", data.fsamp_got);
			printf(")\n");
		}
	}

	if (src)
		src[0]		= data.src;

	if (src_b)
		src_b[0]	= data.src_b;

	if (nrate_a)
		nrate_a[0]	= data.nrate_a;

	if (nrate_b)
		nrate_b[0]	= data.nrate_b;

	if (enable_a)
		enable_a[0]	= data.enable_a;

	if (enable_b)
		enable_b[0]	= data.enable_b;

	if (rate)
		rate[0]		= data.fsamp_got;

	return(errs);
}


