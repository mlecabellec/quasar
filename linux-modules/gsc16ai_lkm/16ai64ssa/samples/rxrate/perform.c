// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/rxrate/perform.c $
// $Rev: 54967 $
// $Date: 2024-08-07 15:57:16 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



// macros *********************************************************************

#define	_2M		(2 * 1024 * 1024)	// size of driver's transfer buffer



// variables ******************************************************************

static	u8	_data[_2M];



//*****************************************************************************
static int _configuration(const args_t* args)
{
	int	errs	= 0;

	errs	+= ai64ssa_config_ai			(args->fd, -1, 1, args->fsamp);
	errs	+= ai64ssa_rx_io_mode			(args->fd, -1, 1, args->io_mode, NULL);
	errs	+= ai64ssa_data_packing			(args->fd, -1, 1, args->data_packing, NULL);
	errs	+= ai64ssa_scan_marker			(args->fd, -1, 1, args->scan_marker, NULL);
	errs	+= ai64ssa_rx_io_overflow		(args->fd, -1, 1, AI64SSA_IO_OVERFLOW_IGNORE, NULL);
	errs	+= ai64ssa_rx_io_underflow		(args->fd, -1, 1, AI64SSA_IO_UNDERFLOW_IGNORE, NULL);
	errs	+= ai64ssa_fsamp_ai_report_all	(args->fd, -1, 1, NULL);
	errs	+= ai64ssa_ai_buf_clear			(args->fd, -1, 1);

	return(errs);
}



//*****************************************************************************
static int _rate_test(const args_t* args, long get)
{
	struct timeval	begin;
	int				errs	= 0;
	long			got;
	long long		limit	= (long long) args->rx_mb * 1000000L;
	struct timeval	minimum;
	struct timeval	now;
	long long		sps;
	long long		total	= 0;
	long			us;

	gsc_label("Reading");

	gettimeofday(&begin, NULL);
	minimum.tv_sec	= begin.tv_sec + args->seconds;
	minimum.tv_usec	= begin.tv_usec;

	for (;;)
	{
		got	= ai64ssa_read(args->fd, _data, get);

		if (got < 0)
		{
			errs++;
			printf("FAIL <--- (read");
			printf(", total so far %lld", total);
			printf(", requested %ld bytes", (long) get);
			printf(", error %ld)\n", got);
			break;
		}

		gettimeofday(&now, NULL);
		total	+= got;

		if (total < limit)
			continue;

		if (now.tv_sec < minimum.tv_sec)
			continue;

		if ((now.tv_sec == minimum.tv_sec) && (now.tv_usec < minimum.tv_usec))
			continue;

		us	= (now.tv_sec - begin.tv_sec) * 1000000L
			+ (now.tv_usec - begin.tv_usec);
		sps	= (total * 10000000L / 4 / us + 5) / 10;

		printf(	"PASS  (");
		gsc_label_long_comma(total / 4);
		printf(" Samples, %ld.%06ld Seconds, ", us / 1000000, us % 1000000);
		gsc_label_long_comma(sps);
		printf(" S/S)\n");
		break;
	}

	return(errs);
}



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int		errs	= 0;
	long	get		= sizeof(_data);

	errs	+= _configuration(args);

	gsc_label ("Read Rate");
	printf("(%d second minimum, %ld MB minimum)\n", args->seconds, (long) args->rx_mb);
	gsc_label_level_inc();

	errs	+= _rate_test(args, get);

	errs	+= ai64ssa_ai_buf_overflow	(args->fd, -1, 1, AI64SSA_AI_BUF_OVERFLOW_CHECK, NULL);
	errs	+= ai64ssa_ai_buf_underflow	(args->fd, -1, 1, AI64SSA_AI_BUF_UNDERFLOW_CHECK, NULL);

	gsc_label_level_dec();
	return(errs);
}



