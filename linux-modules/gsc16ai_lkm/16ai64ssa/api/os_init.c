// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/api/linux/os_init.c $
// $Rev: 42863 $
// $Date: 2018-05-18 12:36:12 -0500 (Fri, 18 May 2018) $

// Linux: API Library: source file

#include "main.h"



// variables ******************************************************************

os_api_global_t	os_api_global;



//*****************************************************************************
// Return Values:
//	0  = success
//	<0 = failure, value is -errno
//	>0 = never returned
int os_api_init(void)
{
	// Nothing special required.
	os_api_global.init	= 1;
	return(0);
}


