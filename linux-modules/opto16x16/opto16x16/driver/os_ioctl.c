// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_ioctl.c $
// $Rev: 49674 $
// $Date: 2021-09-30 22:29:40 -0500 (Thu, 30 Sep 2021) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



// variables ******************************************************************

#if (IOCTL_32BIT_SUPPORT == GSC_IOCTL_32BIT_TRANSLATE)

static	int	_list_qty;

#endif



//*****************************************************************************
static int _common_ioctl(struct file* fp, unsigned int cmd, unsigned long arg)
{
	GSC_ALT_STRUCT_T*	alt;
	int					ret;

	if (fp)
	{
		alt	= (GSC_ALT_STRUCT_T*) fp->private_data;
		ret	= gsc_ioctl(alt, cmd, (void*) arg);
	}
	else
	{
		ret	= -ENODEV;
	}

	return(ret);
}



//*****************************************************************************
#if GSC_HAVE_IOCTL_BKL
int os_ioctl_bkl(
	struct inode*	inode,
	struct file*	fp,
	unsigned int	cmd,
	unsigned long	arg)
{
	int	ret;

	ret	= _common_ioctl(fp, cmd, arg);
	return(ret);
}
#endif



//*****************************************************************************
#if HAVE_COMPAT_IOCTL
long os_ioctl_compat(struct file* fp, unsigned int cmd, unsigned long arg)
{
	int	ret;

	// No additional locking needed as we use a per device lock.
	ret	= _common_ioctl(fp, cmd, arg);
	return(ret);
}
#endif



//*****************************************************************************
#if HAVE_UNLOCKED_IOCTL
long os_ioctl_unlocked(struct file* fp, unsigned int cmd, unsigned long arg)
{
	int	ret;

	// As of 8/13/2008 all data types are sized identically for 32-bit
	// and 64-bit environments.
	// No additional locking needed as we use a per device lock.
	ret	= _common_ioctl(fp, cmd, arg);
	return(ret);
}
#endif



//*****************************************************************************
int os_ioctl_init(void)
{
#if (IOCTL_32BIT_SUPPORT == GSC_IOCTL_32BIT_TRANSLATE)

	int	i;
	int	test;

#endif

	gsc_global.ioctl_32bit	= IOCTL_32BIT_SUPPORT;

#if (IOCTL_32BIT_SUPPORT == GSC_IOCTL_32BIT_TRANSLATE)

	// Compute the size of the IOCTL service list.

	for (i = 0;; i++)
	{
		if ((dev_ioctl_list[i].cmd == -1)	&&
			(dev_ioctl_list[i].func == NULL))
		{
			_list_qty	= i;
			break;
		}
	}

	// Perform the steps needed for support of 32-bit applications
	// under a 64-bit OS.

	// Register each of our defined services.

	for (i = 0; i < _list_qty; i++)
	{
		if (dev_ioctl_list[i].func == NULL)
			continue;

		test	= register_ioctl32_conversion(
					dev_ioctl_list[i].cmd,
					NULL);	// NULL = use 64-bit handler

		if (test)
		{
			// This service has already been registered by
			// someone else.
			printf(	"%s: os_ioctl_init:"
					" Duplicate 'cmd' id at index %d."
					" 32-bit IOCTL support disabled.\n",
					DEV_NAME,
					i);
			gsc_global.ioctl_32bit	= GSC_IOCTL_32BIT_DISABLED;

			// Unregister those that were registered.

			for (; i >= 0; i--)
				unregister_ioctl32_conversion(dev_ioctl_list[i].cmd);

			break;
		}
	}

#endif

	return(0);
}



//*****************************************************************************
void os_ioctl_reset(void)
{
#if (IOCTL_32BIT_SUPPORT == GSC_IOCTL_32BIT_TRANSLATE)
	int	i;

	if (gsc_global.ioctl_32bit)
	{
		for (i = 0; i < _list_qty; i++)
			unregister_ioctl32_conversion(dev_ioctl_list[i].cmd);
	}
#endif
}


