// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_open.c $
// $Rev: 53838 $
// $Date: 2023-11-15 13:51:51 -0600 (Wed, 15 Nov 2023) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



#if (GSC_DEVS_PER_BOARD < 1)
#error "Macro GSC_DEVS_PER_BOARD is improperly defined."
#endif



//*****************************************************************************
int os_open_pre_access(dev_data_t* dev)
{
	// No action required under Linux.
	return(0);
}



//*****************************************************************************
int os_open(struct inode *inode, struct file *fp)
{
	#define	_RW_	(FMODE_READ | FMODE_WRITE)

	GSC_ALT_STRUCT_T*	alt;
	unsigned int		board;
	unsigned int		index	= MINOR(inode->i_rdev);
	int					ret;
	int					share;

	for (;;)	// We'll use a loop for convenience.
	{
		if ((fp->f_mode & _RW_) != _RW_)
		{
			// Read and write access are both required.
			ret	= -EINVAL;
			printf(	"%s: %d. %s: read and write access is required.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		board	= index / GSC_DEVS_PER_BOARD;

		if (board >= SIZEOF_ARRAY(gsc_global.dev_list))
		{
			// The referenced device doesn't exist.
			ret	= -ENODEV;
			printf(	"%s: %d. %s: device index beyond table.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		if (gsc_global.dev_list[board] == NULL)
		{
			// The referenced device doesn't exist.
			ret	= -ENODEV;
			printf(	"%s: %d. %s: device not present.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

#if (GSC_DEVS_PER_BOARD > 1)

		{
			unsigned int	channel;

			channel	= index % GSC_DEVS_PER_BOARD;
			alt		= &gsc_global.dev_list[board]->channel[channel];
		}

#else

		alt	= gsc_global.dev_list[board];

#endif

		share	= (fp->f_flags & O_APPEND) ? 1 : 0;
		ret		= gsc_open(alt, share);

		if (ret == 0)
		{
			ret	= os_module_count_inc();

			if (ret == 0)
			{
				fp->private_data	= alt;
			}
			else
			{
				// We coundn't increase the usage count.
				gsc_close(alt);
			}
		}

		break;
	}

	return(ret);
}



