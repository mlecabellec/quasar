// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/api/gsc_init.c $
// $Rev: 42862 $
// $Date: 2018-05-18 12:34:12 -0500 (Fri, 18 May 2018) $

// OS & Device Independent: API Library: source file

#include "main.h"



//*****************************************************************************
// Return Values:
//	0  = success
//	<0 = failure, value is -errno or -GetLastError(Windows only)
//	>0 = never returned
int gsc_api_init(void)
{
	int	ret;

	ret	= os_api_init();
	return(ret);
}


