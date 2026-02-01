// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/gsc_util_time.c $
// $Rev: 52357 $
// $Date: 2023-01-12 09:58:19 -0600 (Thu, 12 Jan 2023) $

// OS & Device Independent: Utility: source file

#include "main.h"




// macros *********************************************************************

#define	ADD_ITEM(v,t)	sprintf(buf + strlen(buf),		\
								"%d %s%s, ",			\
								(int) (v),				\
								(t),					\
								((v) == 1) ? "" : "s")
#define	ADD_WEEK(v)		ADD_ITEM((v), "Week")
#define	ADD_DAY(v)		ADD_ITEM((v), "Day")
#define	ADD_HOUR(v)		ADD_ITEM((v), "Hr")
#define	ADD_MINUTE(v)	ADD_ITEM((v), "Mn")



/******************************************************************************
*
*	Function:	gsc_time_delta_ms
*
*	Purpose:
*
*		Return a relative change in time in milliseconds. The change in time
*		must be relative to a fixed point in time.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		The change in time in milliseconds.
*
******************************************************************************/

size_t gsc_time_delta_ms(void)
{
	size_t	delta;

	delta	= os_time_delta_ms();
	return(delta);
}



/******************************************************************************
*
*	Function:	gsc_time_format_ms
*
*	Purpose:
*
*		Format a period of time given in milliseconds.
*
*	Arguments:
*
*		ms		The number of milliseconds.
*
*		dest	The formatted value goes here.
*
*		size	This is the size of the above buffer.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_time_format_ms(long ms, char* dest, size_t size)
{
	char	buf[128];
	int		day;
	int		hrs;
	int		min;
	int		sec;
	int		wks;

	sec	= ms / 1000;
	ms	= ms % 1000;

	min	= sec / 60;
	sec	= sec % 60;

	hrs	= min / 60;
	min	= min % 60;

	day	= hrs / 24;
	hrs	= hrs % 24;

	wks	= day / 7;
	day	= day % 7;

	buf[0]	= 0;

	if (wks)
	{
		ADD_WEEK(wks);
		ADD_DAY(day);
		ADD_HOUR(hrs);
		ADD_MINUTE(min);
		sprintf(buf + strlen(buf), "%d.%03d Secs", sec, (int) ms);
	}
	else if (day)
	{
		ADD_DAY(day);
		ADD_HOUR(hrs);
		ADD_MINUTE(min);
		sprintf(buf + strlen(buf), "%d.%03d Secs", sec, (int) ms);
	}
	else if (hrs)
	{
		ADD_HOUR(hrs);
		ADD_MINUTE(min);
		sprintf(buf + strlen(buf), "%d.%03d Secs", sec, (int) ms);
	}
	else if (min)
	{
		ADD_MINUTE(min);
		sprintf(buf + strlen(buf), "%d.%03d Secs", sec, (int) ms);
	}
	else if (sec)
	{
		sprintf(buf + strlen(buf), "%d.%03d Secs", sec, (int) ms);
	}
	else
	{
		sprintf(buf + strlen(buf), "%d ms", (int) ms);
	}

	strncpy(dest, buf, size);
	dest[size - 1]	= 0;
}




/******************************************************************************
*
*	Function:	gsc_time_sleep_ms
*
*	Purpose:
*
*		Sleep for the given number of milliseconds. The OS may sleep longer
*		than requested, much longer, due to factors outside our control.
*
*	Arguments:
*
*		ms	The number of milleseconds to sleep. If zero or less is given we
*			use one.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_time_sleep_ms(long ms)
{
	os_sleep_ms(ms);
}


