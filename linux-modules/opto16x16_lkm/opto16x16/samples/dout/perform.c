// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/dout/perform.c $
// $Rev: 53721 $
// $Date: 2023-09-14 10:39:36 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: source file

#include "main.h"



// macros *********************************************************************

#define	DASH_BITS		0x10101010
#define	DIO_QTY			16
#define	NIBBLE_BITS		0x11111110
#define	ONES			0xFFFF
#define	ZEROS			0x000000



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
	printf("    No.   FEDC-AB98-7654-3210  Time Stamp      Delta         \n");
	printf("    ====  ===================  ==============  ==============\n");
}



//*****************************************************************************
static void _pause_befor_next(const args_t* args)
{
	if (args->pause)
	{
		printf("  Hit return to continue.");
		getchar();
		printf("\n");
	}
}



//*****************************************************************************
static int _dev_dio_out(const args_t* args, u32 value)
{
	static	int				count	= 0;
	static	os_time_ns_t	t0;
	static	os_time_ns_t	ts;
	static	os_time_ns_t	ts_last;

	u32						bit;
	long long				delta;
	int						errs;
	int						i;

	if (count == 0)
	{
		os_time_get_ns(&t0);
		ts		= t0;
		ts_last	= t0;
	}

	ts_last	= ts;
	os_time_get_ns(&ts);
	errs	= opto16x16_tx_data(args->fd, -1, 0, value, NULL);

	count++;
	printf("    %-4d  ", count);

	for (i = DIO_QTY - 1; i >= 0; i--)
	{
		bit	= 0x1 << i;
		printf("%s", (value & bit) ? "1" : "0");

		if (bit & DASH_BITS)
			printf("-");
		else if (bit & NIBBLE_BITS)
			printf(" ");
	}

	if (count == 1)
	{
		t0		= ts;
		ts_last	= ts;
	}

	delta	= (1000000000LL * ts.tv_sec + ts.tv_nsec)
			- (1000000000LL * t0.tv_sec + t0.tv_nsec);
	printf("  %4lld.%09lld", delta / 1000000000, delta % 1000000000);

	delta	= (1000000000LL * ts.tv_sec + ts.tv_nsec)
			- (1000000000LL * ts_last.tv_sec + ts_last.tv_nsec);
	printf("  %4lld.%09lld", delta / 1000000000, delta % 1000000000);

	if (errs)
		printf("  FAIL <---");

	printf("\n");
	fflush(stdout);

	if (args->ms_wait)
		os_sleep_ms(args->ms_wait);

	_pause_befor_next(args);
	return(errs);
}



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int	errs;
	int	i;
	u32	value;

	errs	= _config(args);

	gsc_label("Digital Output");
	printf("\n");
	gsc_label_level_inc();

	_output_header();

	// Flash
	errs	+= _dev_dio_out(args, ONES);
	errs	+= _dev_dio_out(args, ZEROS);
	errs	+= _dev_dio_out(args, ONES);
	errs	+= _dev_dio_out(args, ZEROS);
	errs	+= _dev_dio_out(args, ONES);
	errs	+= _dev_dio_out(args, ZEROS);

	// Walking "1"

	for (i = 0; i < DIO_QTY; i++)
	{
		value	= 1 << i;
		errs	+= _dev_dio_out(args, value);
	}

	// Flash
	errs	+= _dev_dio_out(args, ZEROS);
	errs	+= _dev_dio_out(args, ONES);
	errs	+= _dev_dio_out(args, ZEROS);

	// Walking "0"

	for (i = 0; i < DIO_QTY; i++)
	{
		value	= ~(1L << i) & ONES;
		errs	+= _dev_dio_out(args, value);
	}

	// Flash
	errs	+= _dev_dio_out(args, ONES);
	errs	+= _dev_dio_out(args, ZEROS);
	errs	+= _dev_dio_out(args, ONES);
	errs	+= _dev_dio_out(args, ZEROS);
	errs	+= _dev_dio_out(args, ONES);
	errs	+= _dev_dio_out(args, ZEROS);

	printf("\n");
	gsc_label("Status");
	printf("%s\n", errs ? "FAIL <---" : "PASS");

	gsc_label_level_dec();
	return(errs);
}



