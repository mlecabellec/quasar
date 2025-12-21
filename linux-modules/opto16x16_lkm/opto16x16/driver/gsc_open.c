// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_open.c $
// $Rev: 42883 $
// $Date: 2018-05-29 12:10:25 -0500 (Tue, 29 May 2018) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



//*****************************************************************************
int gsc_open(GSC_ALT_STRUCT_T* alt, int share)
{
	int	ret;

	for (;;)	// A convenience loop.
	{
		ret	= os_sem_lock(&alt->sem);

		if (ret)
		{
			// We didn't get the lock.
			break;
		}

		if (alt->users)
		{
			// The device is already in use.

			if ((share) && (alt->share))
			{
				// The device is already opened in shared mode
				// and we're just gaining access.
				alt->users++;
				os_sem_unlock(&alt->sem);
				break;
			}

			// The desired access cannot be granted.
			ret	= -EBUSY;
			os_sem_unlock(&alt->sem);
			break;
		}

		// Perform device specific OPEN actions.
		ret	= dev_open(alt);

		if (ret)
		{
			// There was a problem here.
			os_sem_unlock(&alt->sem);
			break;
		}

		alt->users	= 1;
		alt->share	= share;
		os_sem_unlock(&alt->sem);
		break;
	}

	return(ret);
}


