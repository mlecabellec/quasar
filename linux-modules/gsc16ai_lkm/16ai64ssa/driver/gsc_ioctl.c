// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_ioctl.c $
// $Rev: 53839 $
// $Date: 2023-11-15 13:53:05 -0600 (Wed, 15 Nov 2023) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



// macros *********************************************************************

#ifndef	OS_IOCTL_XLAT_BY_INDEX_PRE_SIZE
#define	OS_IOCTL_XLAT_BY_INDEX_PRE_SIZE(c,i)
#endif



// variables ******************************************************************

static	int	_list_qty;



//*****************************************************************************
int gsc_ioctl(GSC_ALT_STRUCT_T* alt, unsigned int cmd, void* arg)
{
	u8					buf[512]	= { 0 };
	unsigned long		dir;
	gsc_ioctl_service_t	func;
	int					index;
	unsigned long		ref;
	int					ret;
	unsigned long		size;
	unsigned long		type;

	for (;;)	// A convenience loop.
	{
		if (alt == NULL)
		{
			// The referenced device doesn't exist.
			ret	= -ENODEV;
			printf(	"%s: %d. %s: device pointer is NULL.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		type	= OS_IOCTL_TYPE_DECODE(cmd);
		ref		= GSC_IOCTL;

		if (type != ref)
		{
			// The IOCTL code isn't ours.
			ret	= -EINVAL;
			printf(	"%s: %d. %s: IOCTL type mismatch: expected '%c', received '%c'\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__,
					(char) ref,
					(char) type);
			break;
		}

		index	= OS_IOCTL_INDEX_DECODE(cmd);

		if (index >= _list_qty)
		{
			// The IOCTL service is unrecognized.
			ret	= -EINVAL;
			printf(	"%s: %d. %s: IOCTL index off end of table: %d\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__,
					(int) index);
			break;
		}

		if (dev_ioctl_list[index].func == NULL)
		{
			// The IOCTL service is unimplemented.
			ret	= -EINVAL;
			printf(	"%s: %d. %s: IOCTL function not implemented.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		OS_IOCTL_XLAT_BY_INDEX_PRE_SIZE(cmd, index);
		size	= OS_IOCTL_SIZE_DECODE(cmd);
		ref		= OS_IOCTL_SIZE_DECODE(dev_ioctl_list[index].cmd);

		if (size != ref)
		{
			// The IOCTL data size is invalid.
			ret	= -EINVAL;
			printf(	"%s: %d. %s: IOCTL size mismatch: expected %d, received %d\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__,
					(int) ref,
					(int) size);
			break;
		}

		if ((size) && (arg == NULL))
		{
			// An argument is required but missing.
			ret	= -EINVAL;
			printf(	"%s: %d. %s: IOCTL size calls for argument pointer, which is NULL.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		if (size > sizeof(buf))
		{
			// The buffer is too small.
			ret	= -EFAULT;
			printf(	"%s: %d. %s: IOCTL buffer is too small: limit %d, needed %d\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__,
					(int) sizeof(buf),
					(int) size);
			break;
		}

		dir	= OS_IOCTL_DIR_DECODE(cmd);

		if (dir & (OS_IOCTL_DIR_READ | OS_IOCTL_DIR_WRITE))
		{
			if (arg == NULL)
			{
				// An argument is required but missing.
				ret	= -EINVAL;
				printf(	"%s: %d. %s: IOCTL access calls for argument pointer, which is NULL.\n",
						DEV_NAME,
						__LINE__,
						__FUNCTION__);
				break;
			}
		}

		if (dir & OS_IOCTL_DIR_WRITE)
		{
			ret	= os_mem_copy_from_user_ioctl(buf, arg, size);

			if (ret)
			{
				// There was an error copying from the user's buffer.
				printf(	"%s: %d. %s: copy from app memory failed.\n",
						DEV_NAME,
						__LINE__,
						__FUNCTION__);
				break;
			}
		}

		// Process the IOCTL command.
		ret	= os_sem_lock(&alt->sem);

		if (ret)
		{
			// We didn't get the lock.
			printf(	"%s: %d. %s: failed to acquire lock.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		func	= dev_ioctl_list[index].func;

		if (size)
			ret		= func(alt, buf);
		else
			ret		= func(alt, (void*) arg);

		if ((ret == 0) && (dir & OS_IOCTL_DIR_READ))
		{
			ret	= os_mem_copy_to_user_ioctl(arg, buf, size);

			if (ret)
			{
				// There was an error copying to the user's buffer.
				os_sem_unlock(&alt->sem);
				printf(	"%s: %d. %s: copy to app memory failed.\n",
						DEV_NAME,
						__LINE__,
						__FUNCTION__);
				break;
			}
		}

		os_sem_unlock(&alt->sem);
		break;
	}

	return(ret);
}



//*****************************************************************************
int gsc_ioctl_init(void)
{
	int	ret	= 0;
	int	i;
	int	index;

	// Compute the size of the IOCTL service list.
	_list_qty	= 0;

	for (i = 0;; i++)
	{
		if ((dev_ioctl_list[i].cmd == -1)	&&
			(dev_ioctl_list[i].func == NULL))
		{
			_list_qty	= i;
			break;
		}
	}

	// Verify that the IOCTL service list is in order.

	for (i = 0; i < _list_qty; i++)
	{
		if (dev_ioctl_list[i].func == NULL)
		{
			if (dev_ioctl_list[i].cmd == 0)
				continue;

			printf(	"%s: %d. %s: IOTL function %d not implemented.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__,
					i);
			break;
		}

		index	= OS_IOCTL_INDEX_DECODE(dev_ioctl_list[i].cmd);

		if (index != i)
		{
			ret	= -EIO;
			printf(	"%s: %d. %s: IOCTL entry %d is out of order.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__,
					i);
			break;
		}
	}

	if (ret == 0)
		ret	= os_ioctl_init();

	return(ret);
}



//*****************************************************************************
void gsc_ioctl_reset(void)
{
	os_ioctl_reset();
}


