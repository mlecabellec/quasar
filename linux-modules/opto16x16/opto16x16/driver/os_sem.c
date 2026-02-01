// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_sem.c $
// $Rev: 42879 $
// $Date: 2018-05-29 11:45:41 -0500 (Tue, 29 May 2018) $

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
	}

	return(ret);
}



//*****************************************************************************
void os_sem_destroy(os_sem_t* sem)
{
	if (sem)
		memset(sem, 0, sizeof(os_sem_t));
}



//*****************************************************************************
int os_sem_lock(os_sem_t* sem)
{
	int	ret;

	if ((sem) && (sem->key == sem))
	{
		ret	= down_interruptible(&sem->sem);

		if (ret)
			ret	= -ERESTARTSYS;
	}
	else
	{
		ret	= -EINVAL;
	}

	return(ret);
}



//*****************************************************************************
void os_sem_unlock(os_sem_t* sem)
{
	if ((sem) && (sem->key == sem))
		up(&sem->sem);
}


