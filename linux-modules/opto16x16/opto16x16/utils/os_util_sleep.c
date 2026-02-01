// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/linux/os_util_sleep.c $
// $Rev: 52773 $
// $Date: 2023-04-05 15:53:01 -0500 (Wed, 05 Apr 2023) $

// Linux: Utility: source file

#include "main.h"



//*****************************************************************************
void os_sleep_ms(int ms)
{
	if (ms > 1000)
	{
		for (; ms > 1000; ms -= 1000)
			usleep(1000000);

		if (ms > 0)
			usleep((long) ms * 1000);
	}
	else
	{
		if (ms > 0)
			usleep((long) ms * 1000);
		else
			usleep(0);
	}
}


