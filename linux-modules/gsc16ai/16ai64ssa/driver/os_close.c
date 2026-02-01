// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_close.c $
// $Rev: 53838 $
// $Date: 2023-11-15 13:51:51 -0600 (Wed, 15 Nov 2023) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



//*****************************************************************************
int os_close_post_access(dev_data_t* dev)
{
	// No action required under Linux.
	return(0);
}



//*****************************************************************************
int os_close(struct inode *inode, struct file *fp)
{
	GSC_ALT_STRUCT_T*	alt;
	int					ret;

	for (;;)	// A convenience loop.
	{
		if (fp == NULL)
		{
			// This should never happen.
			ret	= -ENODEV;
			printf(	"%s: %d. %s: 'fp' is NULL.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		alt	= (GSC_ALT_STRUCT_T*) fp->private_data;

		if (alt == NULL)
		{
			// The referenced device doesn't exist.
			ret	= -ENODEV;
			printf(	"%s: %d. %s: 'private_data' is NULL.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		ret	= gsc_close(alt);
		fp->private_data	= NULL;
		os_module_count_dec();
		break;
	}

	return(ret);
}


