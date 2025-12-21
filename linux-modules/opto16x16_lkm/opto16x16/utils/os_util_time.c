// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/linux/os_util_time.c $
// $Rev: 49676 $
// $Date: 2021-09-30 22:35:17 -0500 (Thu, 30 Sep 2021) $

// Linux: Utility: source file

#include "main.h"




//*****************************************************************************
size_t os_time_delta_ms(void)
{
	static	int				started	= 0;
	static	os_time_us_t	start;

	size_t					delta;
	size_t					sec;
	os_time_us_t			tv;
	size_t					us;

	if (started == 0)
	{
		gettimeofday(&start, NULL);
		started	= 1;
	}

	gettimeofday(&tv, NULL);
	sec		= tv.tv_sec - start.tv_sec;
	us		= 1000000L + tv.tv_usec - start.tv_usec;
	delta	= (sec * 1000) + (us / 1000);
	return(delta);
}



//*****************************************************************************
int os_time_get_ns(os_time_ns_t* tns)
{

#ifndef	CLOCK_MONOTONIC_RAW
	os_time_us_t	tus;

	if (tns)
	{
		os_time_get_us(&tus);
		tns->tv_sec		= tus.tv_sec;
		tns->tv_nsec	= tus.tv_usec * 1000;
	}

#else

	#ifdef CLOCK_MONOTONIC
		// This clock is derived from the raw hardware time.
		// NTP attempts to adjust for drift and such.
		#define	CLOCK				CLOCK_MONOTONIC
	#else
		#ifdef CLOCK_MONOTONIC_RAW
			// This clock is based on the raw hardware time.
			// It is never adjusted, so it can drift.
			#define	CLOCK			CLOCK_MONOTONIC_RAW
		#else
			#define	CLOCK			CLOCK_REALTIME
		#endif
	#endif

	clock_gettime(CLOCK, tns);

#endif

	// Always return zero for success as we can't set errno.
	return(0);
}


