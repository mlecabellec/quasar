// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_sem.c $
// $Rev: 53838 $
// $Date: 2023-11-15 13:51:51 -0600 (Wed, 15 Nov 2023) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



//*****************************************************************************
int os_sem_create(os_sem_t* sem)
{
	int	ret;

	if (sem)
	{
		memset(sem, 0, sizeof(os_sem_t));
		sema_init(&sem->sem, 1);
		sem->key	= (void*) sem;
		ret			= 0;
	}
	else
	{
		ret	= -EINVAL;
		printf(	"%s: %d. %s: 'sem' is NULL.\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__);
	}

	return(ret);
}



//*****************************************************************************
void os_sem_destroy(os_sem_t* sem)
{
	if (sem)
	{
		memset(sem, 0, sizeof(os_sem_t));
	}
	else
	{
		// We don't report an error message here. This situation can arise if
		// things are being shut down due to some other failure.
	}
}



//*****************************************************************************
int os_sem_lock(os_sem_t* sem)
{
	int	ret	= 0;

	if ((sem) && (sem->key == sem))
	{
		ret	= down_interruptible(&sem->sem);

		if (ret)
		{
			ret	= -ERESTARTSYS;
			printf(	"%s: %d. %s: failed to acquire lock.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
		}
	}
	else
	{
		// We don't report an error message here. This situation can arise if
		// things are being shut down due to some other failure.
	}

	return(ret);
}



//*****************************************************************************
void os_sem_unlock(os_sem_t* sem)
{
	if ((sem) && (sem->key == sem))
	{
		up(&sem->sem);
	}
	else
	{
		// We don't report an error message here. This situation can arise if
		// things are being shut down due to some other failure.
	}
}


