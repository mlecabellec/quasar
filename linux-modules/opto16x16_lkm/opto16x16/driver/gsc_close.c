// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_close.c $
// $Rev: 42883 $
// $Date: 2018-05-29 12:10:25 -0500 (Tue, 29 May 2018) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



//*****************************************************************************
int gsc_close(GSC_ALT_STRUCT_T* alt)
{
	int	ret;
	int	tmp	= 0;

	ret	= os_sem_lock(&alt->sem);

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


