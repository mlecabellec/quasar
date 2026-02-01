// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_spinlock.c $
// $Rev: 42879 $
// $Date: 2018-05-29 11:45:41 -0500 (Tue, 29 May 2018) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



/******************************************************************************
*
*	Function:	os_spinlock_create
*
*	Purpose:
*
*		Create a spin lock.
*
*	Arguments:
*
*		lock	The OS specific data type for the lock.
*
*	Returned:
*
*		0		All went well.
*		< 0		An appropriate error status.
*
******************************************************************************/

int os_spinlock_create(os_spinlock_t* lock)
{
	if (lock)
	{
		memset(lock, 0, sizeof(os_spinlock_t));
		spin_lock_init(&lock->lock);
		lock->key	= lock;
	}

	return(0);
}



/******************************************************************************
*
*	Function:	os_spinlock_destroy
*
*	Purpose:
*
*		Destroy a spin lock.
*
*	Arguments:
*
*		lock	The OS specific data type for the lock.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void os_spinlock_destroy(os_spinlock_t* lock)
{
	if (lock)
		memset(lock, 0, sizeof(os_spinlock_t));
}



/******************************************************************************
*
*	Function:	os_spinlock_lock
*
*	Purpose:
*
*		Acquire a spin lock.
*
*	Arguments:
*
*		lock	The OS specific data type for the lock.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void os_spinlock_lock(os_spinlock_t* lock)
{
	unsigned long	flags	= 0;

	if ((lock) && (lock->key == lock))
	{
		spin_lock_irqsave(&lock->lock, flags);
		lock->flags	= flags;
	}
}



/******************************************************************************
*
*	Function:	os_spinlock_unlock
*
*	Purpose:
*
*		Release a spin lock.
*
*	Arguments:
*
*		lock	The OS specific data type for the lock.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void os_spinlock_unlock(os_spinlock_t* lock)
{
	unsigned long	flags;

	if ((lock) && (lock->key == lock))
	{
		flags	= lock->flags;
		spin_unlock_irqrestore(&lock->lock, flags);
	}
}


