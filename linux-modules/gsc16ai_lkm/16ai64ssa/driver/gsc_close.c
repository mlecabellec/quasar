// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_close.c $
// $Rev: 53839 $
// $Date: 2023-11-15 13:53:05 -0600 (Wed, 15 Nov 2023) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



//*****************************************************************************
int gsc_close(GSC_ALT_STRUCT_T* alt)
{
	int	ret;
	int	tmp	= 0;

	ret	= os_sem_lock(&alt->sem);

	if (ret)
	{
		printf(	"%s: %d. %s: failed to acquire lock.\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__);
	}

	if (alt->users)
		alt->users--;

	if (alt->users == 0)
		tmp	= dev_close(alt);

	if (ret == 0)
	{
		ret	= tmp;
		os_sem_unlock(&alt->sem);
	}

	return(ret);
}


