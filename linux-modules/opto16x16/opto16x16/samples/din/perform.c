// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/din/perform.c $
// $Rev: 53720 $
// $Date: 2023-09-14 10:39:22 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: source file

#include "main.h"



// macros *********************************************************************

#define	DASH_BITS		0x10101010
#define	DIO_BITS		16
#define	NIBBLE_BITS		0x11111110



//*****************************************************************************
static int _config(const args_t* args)
{
	int	errs	= 0;

	gsc_label("Configuration");
	printf("\n");
	gsc_label_level_inc();

	errs	+= opto16x16_initialize(args->fd, -1, 1);

	gsc_label_level_dec();
	return(errs);
}



//*****************************************************************************
static void _output_header(void)
{
	printf("\n");
	printf("    Seq   D15-D8    D7-D0 \n");
	printf("    No.   FEDC-BA98-7654-3210  Time Stamp      Delta         \n");
	printf("    ====  ===================  ==============  ==============\n");
}



//*****************************************************************************
static int _monitor(const args_t* args)
{
	u32				bit;
	int				count	= 0;
	long long		delta;
	int				errs	= 0;
	int				i;
	s32				last	= -1;
	time_t			now;
	time_t			t_limit;
	os_time_ns_t	t0;
	os_time_ns_t	ts;
	os_time_ns_t	ts_last;
	s32				value;

	gsc_label("Digital Input");
	printf("\n");
	gsc_label_level_inc();

	os_time_get_ns(&t0);
	os_time_get_ns(&ts);

	_output_header();
	t_limit	= time(NULL) + args->seconds + 1;

	for (; errs == 0;)
	{
		errs	= opto16x16_rx_data(args->fd, -1, 0, &value);

		if ((count <= 0) || (value != last))
		{
			ts_last	= ts;
			os_time_get_ns(&ts);

			if (count <= 0)
			{
				os_time_get_ns(&t0);
				ts		= t0;
				ts_last	= t0;
			}

			count++;
			printf("    %-4d  ", count);

			for (i = DIO_BITS - 1; i >= 0; i--)
			{
				bit	= 0x1 << i;
				printf("%s", (value & bit) ? "1" : "0");

				if (bit & DASH_BITS)
					printf("-");
				else if (bit & NIBBLE_BITS)
					printf(" ");
			}

			delta	= (1000000000LL * ts.tv_sec + ts.tv_nsec)
					- (1000000000LL * t0.tv_sec + t0.tv_nsec);
			printf("  %4lld.%09lld", delta / 1000000000, delta % 1000000000);

			delta	= (1000000000LL * ts.tv_sec + ts.tv_nsec)
					- (1000000000LL * ts_last.tv_sec + ts_last.tv_nsec);
			printf("  %4lld.%09lld", delta / 1000000000, delta % 1000000000);

			printf("\n");
			fflush(stdout);
			last	= value;
		}

		now	= time(NULL);

		if (now >= t_limit)
			break;

		if (args->ms_wait)
			os_sleep_ms(args->ms_wait);
	}

	printf("\n");
	gsc_label("Status");
	printf("%s\n", errs ? "FAIL <---" : "PASS");

	gsc_label_level_dec();
	return(errs);
}



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int	errs	= 0;

	errs	+= _config(args);
	errs	+= _monitor(args);

	return(errs);
}


