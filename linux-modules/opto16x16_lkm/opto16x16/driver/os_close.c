// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_close.c $
// $Rev: 42879 $
// $Date: 2018-05-29 11:45:41 -0500 (Tue 29 May 2018) $

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
			break;
		}

		alt	= (GSC_ALT_STRUCT_T*) fp->private_data;

		if (alt == NULL)
		{
			// The referenced device doesn't exist.
			ret	= -ENODEV;
			break;
		}

		ret	= gsc_close(alt);
		fp->private_data	= NULL;
		os_module_count_dec();
		break;
	}

	return(ret);
}


