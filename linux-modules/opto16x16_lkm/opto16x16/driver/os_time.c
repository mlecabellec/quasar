// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_time.c $
// $Rev: 49674 $
// $Date: 2021-09-30 22:29:40 -0500 (Thu, 30 Sep 2021) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



//*****************************************************************************
long os_time_delta_ms(const os_time_t* tt1, const os_time_t* tt2)
{
	long	ms;
	long	us;

	if (tt1->tv_sec == tt2->tv_sec)
	{
		us	= (long) (tt2->tv_usec - tt1->tv_usec);
		ms	= (us + 999) / 1000;
	}
	else if (tt1->tv_sec < tt2->tv_sec)
	{
		us	= (long) (tt2->tv_usec + 1000000 - tt1->tv_usec);
		ms	= (us + 999) / 1000;
		ms	+= (long) ((tt2->tv_sec - tt1->tv_sec - 1) * 1000);
	}
	else // if (tt1->tv_sec > tt2->tv_sec)
	{
		ms	= -os_time_delta_ms(tt2, tt1);
	}

	return(ms);
}



//*****************************************************************************
void os_time_get(os_time_t* tt)
{
	do_gettimeofday(tt);
}



//*****************************************************************************
os_time_tick_t os_time_ms_to_ticks(long ms)
{
	os_time_tick_t	ticks;

	ticks	= (ms + ((1000 / HZ) - 1)) / (1000 / HZ);
	return(ticks);
}



//*****************************************************************************
os_time_tick_t os_time_tick_get(void)
{
	return(jiffies);
}



//*****************************************************************************
int os_time_tick_rate(void)
{
	return(HZ);
}



//*****************************************************************************
int os_time_tick_sleep(int ticks)
{
	int	ret;

	if (ticks < 0)
		ticks	= 0;

	SET_CURRENT_STATE(TASK_INTERRUPTIBLE);
	ret	= schedule_timeout(ticks);
	SET_CURRENT_STATE(TASK_RUNNING);

	if (ret)
	{
		// The timeout returned prematurely; it was signaled.
		// We've essentially been told to quit.
		ret	= -ECANCELED;
	}

	return(ret);
}



//*****************************************************************************
int os_time_tick_timedout(os_time_tick_t tick_time)
{
	int	timedout;

	timedout	= time_after(jiffies, tick_time);
	return(timedout);
}



//*****************************************************************************
long os_time_ticks_to_ms(os_time_tick_t ticks)
{
	long	ms;

	ms	= (((ticks) * 1000) + (HZ / 2)) / HZ;
	return(ms);
}



//*****************************************************************************
void os_time_us_delay(long us)
{
	udelay(us);
}



//*****************************************************************************
int os_time_sleep_ms(long ms)
{
	os_time_tick_t	limit	= os_time_tick_get() + os_time_ms_to_ticks(ms);
	int				ret		= 0;

	for (;;)
	{
		if (os_time_tick_timedout(limit))
		{
			// We've waited long enough.
			break;
		}

		ret	= os_time_tick_sleep(1);

		if (ret)
		{
			// The timeout returned prematurely; it was signaled.
			// We've essentially been told to quit.
			break;
		}
	}

	return(ret);
}



