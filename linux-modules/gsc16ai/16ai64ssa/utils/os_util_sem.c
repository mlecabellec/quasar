// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/linux/os_util_sem.c $
// $Rev: 46870 $
// $Date: 2020-02-24 12:41:55 -0600 (Mon, 24 Feb 2020) $

// Linux: Utility: source file

#include "main.h"



//*****************************************************************************
int os_sem_create(os_sem_t* sem)
{
	int	ret;

	ret	= os_sem_create_qty(sem, 1, 1);
	return(ret);
}



//*****************************************************************************
int os_sem_create_qty(os_sem_t* sem, int cap, int put)
{
	int	ret;

	if ((sem) && (cap > 0) && (put >= 0) && (put <= cap))
	{
		memset(sem, 0, sizeof(os_sem_t));
		ret	= sem_init(&sem->sem, 0, put);

		if (ret)
			ret	= -errno;
		else
			sem->key	= (void*) sem;
	}
	else
	{
		ret	= -EINVAL;
	}

	return(ret);
}



//*****************************************************************************
int os_sem_destroy(os_sem_t* sem)
{
	int	ret	= 0;

	if (sem)
	{
		if (sem->key == sem)
		{
			os_sem_unlock(sem);
			ret	= sem_destroy(&sem->sem);

			if (ret)
				ret	= -errno;
		}

		memset(sem, 0, sizeof(os_sem_t));
	}
	else
	{
		ret	= -EINVAL;
	}

	return(ret);
}



//*****************************************************************************
int os_sem_lock(os_sem_t* sem)
{
	int	ret;

	if ((sem) && (sem->key == sem))
	{
		ret	= sem_wait(&sem->sem);

		if (ret)
			ret	= -errno;
	}
	else
	{
		ret	= -EINVAL;
	}

	return(ret);
}



//*****************************************************************************
int os_sem_unlock(os_sem_t* sem)
{
	int	ret;

	if ((sem) && (sem->key == sem))
	{
		ret	= sem_post(&sem->sem);

		if (ret)
			ret	= -errno;
	}
	else
	{
		ret	= -EINVAL;
	}

	return(ret);
}


